#include "lisp.h"
#include "symbols.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Statistics.
 */

typedef struct _stats_t
{
  size_t  n_alloc;
  size_t  n_free;
}
stats_t;

static stats_t stats = { 0 };

void
lisp_stats_print(FILE * fp)
{
  fprintf(fp, "[STATS] alloc:%ld/free:%ld\n", stats.n_alloc, stats.n_free);
}

bool
lisp_stats_balanced_allocs()
{
  return stats.n_alloc == stats.n_free;
}

size_t
lisp_stats_get_alloc()
{
  return stats.n_alloc;
}

size_t
lisp_stats_get_free()
{
  return stats.n_free;
}

#define ALLOCATE(__p) {                                       \
  posix_memalign((void **)&__p, 16, sizeof(struct _cell_t));  \
  memset(__p, 0, sizeof(struct _cell_t));                     \
  stats.n_alloc += 1;                                         \
}

#define FREE(__p) {   \
  free(__p);          \
  stats.n_free += 1;  \
}

/*
 * Basic functions.
 */

static uintptr_t
slot_dup(const uintptr_t slot)
{
  uintptr_t result = slot;
  switch (GET_TYPE(slot)) {
    case T_NIL:
    case T_TRUE:
    case T_NUMBER:
    case T_SYMBOL_INLINE:
      break;
    case T_STRING:
    case T_SYMBOL:
      SET_DATA(result, strdup(GET_PNTR(char *, slot)));
      break;
    case T_LIST:
      SET_DATA(result, lisp_dup(GET_PNTR(cell_t, slot)));
      break;
  }
  return result;
}

cell_t
lisp_dup(const cell_t cell)
{
  cell_t R = NULL;
  ALLOCATE(R);
  R->car = slot_dup(cell->car);
  R->cdr = slot_dup(cell->cdr);
  return R;
}

cell_t
lisp_car(const cell_t cell)
{
  if (!IS_LIST(cell)) {
    return lisp_make_nil();
  }
  cell_t L = GET_PNTR(cell_t, cell->car);
  return lisp_make_slot(L->car);
}

cell_t
lisp_cdr(const cell_t cell)
{
  if (!IS_LIST(cell)) {
    return lisp_make_nil();
  }
  cell_t L = GET_PNTR(cell_t, cell->car);
  return lisp_make_slot(L->cdr);
}

size_t
lisp_len(const cell_t cell)
{
  size_t result = IS_NULL(cell) ? 0 : 1;
  if (IS_LIST(cell)) {
    cell_t p = GET_PNTR(cell_t, cell->car);
    while (GET_TYPE(p->cdr) == T_LIST) {
      result += 1;
      p = GET_PNTR(cell_t, p->cdr);
    }
  }
  return result;
}

/*
 * Internal list construction functions.
 */

static bool
slot_equl(const uintptr_t a, const uintptr_t b)
{
  if (GET_TYPE(a) != GET_TYPE(b)) {
    return false;
  }
  switch (GET_TYPE(a)) {
    case T_NIL:
    case T_TRUE:
      return true;
    case T_NUMBER:
      return GET_NUMB(a) == GET_NUMB(b);
    case T_STRING:
    case T_SYMBOL:
      return strcmp(GET_PNTR(char *, a), GET_PNTR(char *, b)) == 0;
    case T_SYMBOL_INLINE:
      return strcmp(GET_SYMB(a), GET_SYMB(b)) == 0;
    case T_LIST:
      return lisp_equl(GET_PNTR(cell_t, a), GET_PNTR(cell_t, b));
  }
}

bool
lisp_equl(const cell_t a, const cell_t b)
{
  return slot_equl(a->car, b->car) && slot_equl(a->cdr, b->cdr);
}

cell_t
lisp_cons(const cell_t a, const cell_t b)
{
  cell_t n = lisp_dup(a), p = n;
  while (GET_TYPE(p->cdr) == T_LIST) {
    p = GET_PNTR(cell_t, p->cdr);
  }
  p->cdr = slot_dup(b->car);
  cell_t result = lisp_make_list(n);
  lisp_free(1, n);
  return result;
}

cell_t
lisp_conc(const cell_t a, const cell_t b)
{
  if (!IS_LIST(a)) {
    lisp_free(1, a);
    return b;
  }
  cell_t p = GET_PNTR(cell_t, a->car);
  while (GET_TYPE(p->cdr) == T_LIST) {
    p = GET_PNTR(cell_t, p->cdr);
  }
  p->cdr = b->car;
  FREE(b);
  return a;
}

static cell_t
lisp_eval_args(const cell_t cell)
{
  /*
   * Return NIL if NIL.
   */
  if (GET_TYPE(cell->car) != T_LIST) {
    return cell;
  }
  /*
   * Get CAR and CDR.
   */
  cell_t car = lisp_car(cell);
  cell_t cdr = lisp_cdr(cell);
  /*
   * Eval CAR and CDR, and CONS the result.
   */
  cell_t rar = lisp_eval(car);
  cell_t rdr = lisp_eval_args(cdr);
  cell_t res = lisp_cons(rar, rdr);
  /*
   * Clean-up and return the CONSed result.
   */
  lisp_free(3, cell, rar, rdr);
  return res;
}

static cell_t
lisp_eval_list(const cell_t cell)
{
  cell_t res = NULL;
  cell_t car = lisp_car(cell);
  cell_t cdr = lisp_cdr(cell);
  /*
   * Process CAR.
   */
  char * sym = NULL;
  switch (GET_TYPE(car->car)) {
    case T_SYMBOL:
      sym = GET_PNTR(char *, car->car);
      break;
    case T_SYMBOL_INLINE:
      sym = GET_SYMB(car->car);
      break;
    default: break;
  }
  /*
   * Call the function.
   */
  uintptr_t fun = 0;
  if (sym == NULL || (fun = lisp_symbol_lookup(sym)) == 0) {
    res = lisp_make_nil();
    lisp_free(1, cdr);
  }
  /*
   * Find a better way to handle this.
   */
  else {
    if (IS_EVAL(fun)) {
      cdr = lisp_eval_args(cdr);
    }
    if (IS_FUNC(fun)) {
      res = GET_PNTR(function_t, fun)(cdr);
    }
    /*
     * FIXME Add support for lambda evaluation.
     */
    else {
      res = lisp_make_nil();
      lisp_free(1, cdr);
    }
  }
  /*
   * Clean-up;
   */
  lisp_free(2, car, cell);
  return res;
}

cell_t
lisp_eval(const cell_t cell)
{
  switch (GET_TYPE(cell->car)) {
    case T_NIL:
    case T_TRUE:
    case T_NUMBER:
    case T_STRING: {
      return cell;
    }
    case T_SYMBOL: {
      char * sym = GET_PNTR(char *, cell->car);
      lisp_free(1, cell);
      uintptr_t fun = lisp_symbol_lookup(sym);
      if (fun == 0) return lisp_make_nil();
      return lisp_make_number(fun);
      break;
    }
    case T_SYMBOL_INLINE: {
      char * sym = GET_SYMB(cell->car);
      lisp_free(1, cell);
      uintptr_t fun = lisp_symbol_lookup(sym);
      if (fun == 0) return lisp_make_nil();
      return lisp_make_number(fun);
    }
    case T_LIST: {
      return lisp_eval_list(cell);
    }
  }
}

/*
 * Helper functions.
 */

cell_t
lisp_make_nil()
{
  cell_t R = NULL;
  ALLOCATE(R);
  return R;
}

cell_t
lisp_make_true()
{
  cell_t R = NULL;
  ALLOCATE(R);
  SET_TYPE(R->car, T_TRUE);
  return R;
}

cell_t
lisp_make_number(const uint64_t num)
{
  cell_t R = NULL;
  ALLOCATE(R);
  SET_TYPE(R->car, T_NUMBER);
  SET_NUMB(R->car, num);
  return R;
}

cell_t
lisp_make_string(const char * const str)
{
  cell_t R = NULL;
  ALLOCATE(R);
  SET_TYPE(R->car, T_STRING);
  SET_DATA(R->car, strdup(str));
  return R;
}

cell_t
lisp_make_symbol(const char * const sym)
{
  cell_t R = NULL;
  ALLOCATE(R);
  /*
   * Make the symbol inline if possible.
   */
  if (strlen(sym) < INLINE_SYMBOL_LEN) {
    SET_TYPE(R->car, T_SYMBOL_INLINE);
    SET_SYMB(R->car, sym);
  }
  else {
    SET_TYPE(R->car, T_SYMBOL);
    SET_DATA(R->car, strdup(sym));
  }
  return R;
}

cell_t
lisp_make_list(const cell_t cell)
{
  cell_t R = NULL;
  cell_t N = lisp_dup(cell);
  ALLOCATE(R);
  SET_TYPE(R->car, T_LIST);
  SET_DATA(R->car, N);
  return R;
}

cell_t
lisp_make_slot(const uintptr_t slot)
{
  cell_t R = NULL;
  ALLOCATE(R);
  R->car = slot_dup(slot);
  return R;
}

static void
lisp_free_entry(const uintptr_t entry)
{
  switch (GET_TYPE(entry)) {
    case T_LIST: {
      lisp_free(1, GET_PNTR(cell_t, entry));
      break;
    }
    case T_STRING:
    case T_SYMBOL: {
      free(GET_PNTR(char *, entry));
      break;
    }
    default: break;
  }
}

void
lisp_free(const size_t n, ...)
{
  va_list args;
  va_start(args, n);
  /*
   * Deleting the items.
   */
  for (size_t i = 0; i < n; i += 1) {
    cell_t cell = va_arg(args, cell_t);
    lisp_free_entry(cell->car);
    lisp_free_entry(cell->cdr);
    FREE(cell);
  }
  /*
   * Clean-up.
   */
  va_end(args);
}
