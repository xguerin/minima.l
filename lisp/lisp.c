#include "lisp.h"
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

#define ALLOCATE(__p) {                                          \
  posix_memalign((void **)&__p, 16, sizeof(struct _cell_t));  \
  memset(__p, 0, sizeof(struct _cell_t));                     \
  stats.n_alloc += 1;                                         \
}

#define FREE(__p) {   \
  free(__p);          \
  stats.n_free += 1;  \
}

/*
 * Function types.
 */

static function_entry_t function_table = { NULL, NULL };

function_t
lisp_function_lookup(const char * const sym)
{
  return NULL;
}

bool
lisp_function_register(const char * const sym, function_t fun)
{
  size_t len = strlen(sym);
  /*
   * FIXME For the moment, restrict to the fast lookup table.
   */
  if (len > FUNCTION_TABLE_LVL) {
    return false;
  }
  /*
   * Get the function entry.
   */
  function_entry_t * entry = &function_table;
  for (size_t i = 0; i < len; i += 1) {
    /*
     * Allocate the new level.
     */
    if (entry->table == NULL) {
      posix_memalign((void **)&entry->table, 8, FUNCTION_TABLE_LEN *
                     sizeof(function_entry_t));
      memset(entry->table, 0, FUNCTION_TABLE_LEN * sizeof(function_entry_t));
    }
    /*
     * Grab the new entry.
     */
    entry = &entry->table[(size_t)sym[i]];
  }
  /*
   * Register the function.
   */
  entry->fun = fun;
  return true;
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
  size_t result = 1;
  if (IS_LIST(cell)) {
    result += lisp_len(GET_PNTR(cell_t, cell->cdr));
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
    return lisp_make_nil();
  }
  cell_t p = GET_PNTR(cell_t, a->car);
  while (GET_TYPE(p->cdr) == T_LIST) {
    p = GET_PNTR(cell_t, p->cdr);
  }
  p->cdr = b->car;
  FREE(b);
  return a;
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

static void lisp_print_cell(FILE * const fp, const cell_t cell);
static void lisp_print_list(FILE * const fp, const cell_t cell);

static void lisp_print_list(FILE * const fp, const cell_t cell)
{
  /*
   * Process CAR.
   */
  switch (GET_TYPE(cell->car)) {
    case T_NUMBER: {
      fprintf(fp, "%lld", GET_NUMB(cell->car));
      break;
    }
    case T_STRING: {
      fprintf(fp, "\"%s\"", GET_PNTR(char *, cell->car));
      break;
    }
    case T_SYMBOL: {
      fprintf(fp, "%s", GET_PNTR(char *, cell->car));
      break;
    }
    case T_SYMBOL_INLINE: {
      fprintf(fp, "%s", GET_SYMB(cell->car));
      break;
    }
    case T_LIST: {
      fputc('(', fp);
      lisp_print_cell(fp, GET_PNTR(cell_t, cell->car));
      fputc(')', fp);
      break;
    }
    case T_NIL : {
      printf("NIL");
      break;
    }
  }
  /*
   * Process CDR.
   */
  switch (GET_TYPE(cell->cdr)) {
    case T_NUMBER: {
      fprintf(fp, " . %lld", GET_NUMB(cell->cdr));
      break;
    }
    case T_STRING: {
      fprintf(fp, " . \"%s\"", GET_PNTR(char *, cell->cdr));
      break;
    }
    case T_SYMBOL: {
      fprintf(fp, " . %s", GET_PNTR(char *, cell->cdr));
      break;
    }
    case T_SYMBOL_INLINE: {
      fprintf(fp, " . %s", GET_SYMB(cell->cdr));
      break;
    }
    case T_LIST: {
      fprintf(fp, " ");
      lisp_print_list(fp, GET_PNTR(cell_t, cell->cdr));
      break;
    }
    case T_NIL : {
      break;
    }
  }
}

static void lisp_print_cell(FILE * const fp, const cell_t cell)
{
  lisp_print_list(fp, cell);
}

void
lisp_print(FILE * const fp, const cell_t cell)
{
  lisp_print_cell(fp, cell);
  printf("\n");
}
