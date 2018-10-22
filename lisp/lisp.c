#include "functions.h"
#include "lisp.h"
#include "slab.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Symbol management.
 */

cell_t globals;

char *
lisp_get_sym(const cell_t cell)
{
  if (GET_TYPE(cell->car) == T_SYMBOL) {
    return GET_PNTR(char *, cell->car);
  }
  if (GET_TYPE(cell->car) == T_SYMBOL_INLINE) {
    return GET_SYMB(cell->car);
  }
  return NULL;
}

cell_t
lisp_lookup(const cell_t sym)
{
  FOREACH(globals, p) {
    cell_t car = GET_PNTR(cell_t, p->car);
    if (strcmp(lisp_get_sym(car), lisp_get_sym(sym)) == 0) {
      return lisp_cdr(p);
    }
    NEXT(p);
  }
  return lisp_make_nil();
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
  cell_t R = lisp_allocate();
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
  slot_free(p->cdr);
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
  FOREACH(a, p) NEXT(p);
  slot_free(p->cdr);
  p->cdr = b->car;
  lisp_deallocate(b);
  return a;
}

cell_t
lisp_setq(const cell_t a, const cell_t b)
{
  cell_t con = lisp_cons(a, b);
  /*
   * Check if the symbol exists and replace it. We use a zero-copy algorithm
   * here for obvious performance reasons.
   */
  FOREACH(globals, p) {
    cell_t car = GET_PNTR(cell_t, p->car);
    if (strcmp(lisp_get_sym(car), lisp_get_sym(a)) == 0) {
      lisp_replace(p, con);
      return b;
    }
    NEXT(p);
  }
  /*
   * The symbol does not exist, so append it.
   */
  cell_t lst = lisp_make_list(con);
  globals = lisp_conc(globals, lst);
  lisp_free(1, con);
  return b;
}

static cell_t
lisp_eval_args(const cell_t cell)
{
  TRACE(cell);
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
  TRACE(cell);
  /*
   */
  cell_t res = NULL;
  cell_t car = lisp_car(cell);
  cell_t cdr = lisp_cdr(cell);
  /*
   * Check what the first argument is.
   */
  switch (GET_TYPE(car->car)) {
    case T_SYMBOL:
    case T_SYMBOL_INLINE:
    case T_LIST:
      car = lisp_eval(car);
      break;
    default:
      res = lisp_dup(cell);
      lisp_free(3, cell, car, cdr);
      return res;
  }
  /*
   * Process the result. FIXME Add support for lambda evaluation.
   */
  if (GET_TYPE(car->car) != T_NUMBER) {
      res = lisp_make_nil();
      lisp_free(3, cell, car, cdr);
      return res;
  }
  uintptr_t fun = GET_NUMB(car->car);
  /*
   * Call the function. May SIGSEV.
   */
  if (IS_EVAL(fun)) {
    cdr = lisp_eval_args(cdr);
  }
  res = GET_PNTR(function_t, fun)(cdr);
  /*
   * Clean-up;
   */
  lisp_free(2, car, cell);
  return res;
}

cell_t
lisp_eval(const cell_t cell)
{
  TRACE(cell);
  /*
   */
  switch (GET_TYPE(cell->car)) {
    case T_NIL:
    case T_TRUE:
    case T_NUMBER:
    case T_STRING: {
      return cell;
    }
    case T_SYMBOL:
    case T_SYMBOL_INLINE: {
      cell_t res = lisp_lookup(cell);
      lisp_free(1, cell);
      return res;
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
  return lisp_allocate();
}

cell_t
lisp_make_true()
{
  cell_t R = lisp_allocate();
  SET_TYPE(R->car, T_TRUE);
  return R;
}

cell_t
lisp_make_number(const uint64_t num)
{
  cell_t R = lisp_allocate();
  SET_TYPE(R->car, T_NUMBER);
  SET_NUMB(R->car, num);
  return R;
}

cell_t
lisp_make_string(const char * const str)
{
  cell_t R = lisp_allocate();
  SET_TYPE(R->car, T_STRING);
  SET_DATA(R->car, strdup(str));
  return R;
}

cell_t
lisp_make_symbol(const char * const sym)
{
  cell_t R = lisp_allocate();
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
  cell_t R = lisp_allocate();
  cell_t N = lisp_dup(cell);
  SET_TYPE(R->car, T_LIST);
  SET_DATA(R->car, N);
  return R;
}

cell_t
lisp_make_slot(const uintptr_t slot)
{
  cell_t R = lisp_allocate();
  R->car = slot_dup(slot);
  return R;
}
