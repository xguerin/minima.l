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

cell_t GLOBALS = NULL;

static char *
lisp_get_sym_noop(const cell_t cell)
{
  return NULL;
}

static char *
lisp_get_sym_pntr(const cell_t cell)
{
  return GET_PNTR(char *, cell->car);
}

static char *
lisp_get_sym_inline(const cell_t cell)
{
  return GET_SYMB(cell->car);
}

static char * (* lisp_get_sym_table[8])(const cell_t cell) =
{
  [T_NIL          ] = lisp_get_sym_noop,
  [T_LIST         ] = lisp_get_sym_noop,
  [T_NUMBER       ] = lisp_get_sym_noop,
  [T_STRING       ] = lisp_get_sym_noop,
  [T_SYMBOL       ] = lisp_get_sym_pntr,
  [T_SYMBOL_INLINE] = lisp_get_sym_inline,
  [T_TRUE         ] = lisp_get_sym_noop,
  [T_WILDCARD     ] = lisp_get_sym_noop,
};

static char *
lisp_get_sym(const cell_t cell)
{
  return lisp_get_sym_table[GET_TYPE(cell->car)](cell);
}

static cell_t
lisp_lookup(const cell_t closure, const cell_t sym)
{
  FOREACH(closure, a) {
    cell_t car = GET_PNTR(cell_t, a->car);
    if (strcmp(lisp_get_sym(car), lisp_get_sym(sym)) == 0) {
      return lisp_cdr(a);
    }
    NEXT(a);
  }
  FOREACH(GLOBALS, b) {
    cell_t car = GET_PNTR(cell_t, b->car);
    if (strcmp(lisp_get_sym(car), lisp_get_sym(sym)) == 0) {
      return lisp_cdr(b);
    }
    NEXT(b);
  }
  return lisp_make_nil();
}

/*
 * Basic functions.
 */

static uintptr_t
slot_dup_noop(const uintptr_t slot)
{
  return slot;
}

static uintptr_t
slot_dup_string(const uintptr_t slot)
{
  uintptr_t result = slot;
  SET_DATA(result, strdup(GET_PNTR(char *, slot)));
  return result;
}

static uintptr_t
slot_dup_list(const uintptr_t slot)
{
  uintptr_t result = slot;
  SET_DATA(result, lisp_dup(GET_PNTR(cell_t, slot)));
  return result;
}

static uintptr_t (* slot_dup_table[8])(const uintptr_t) =
{
  [T_NIL          ] = slot_dup_noop,
  [T_LIST         ] = slot_dup_list,
  [T_NUMBER       ] = slot_dup_noop,
  [T_STRING       ] = slot_dup_string,
  [T_SYMBOL       ] = slot_dup_string,
  [T_SYMBOL_INLINE] = slot_dup_noop,
  [T_TRUE         ] = slot_dup_noop,
  [T_WILDCARD     ] = slot_dup_noop,
};

static uintptr_t
slot_dup(const uintptr_t slot)
{
  return slot_dup_table[GET_TYPE(slot)](slot);
}

cell_t
lisp_dup(const cell_t cell)
{
  cell_t R = lisp_allocate();
  R->car = slot_dup(cell->car);
  R->cdr = slot_dup(cell->cdr);
  return R;
}

static cell_t
lisp_car_list(const cell_t cell)
{
  cell_t L = GET_PNTR(cell_t, cell->car);
  return lisp_make_slot(L->car);
}

static cell_t
lisp_car_noop(const cell_t cell)
{
  return lisp_make_nil();
}

static cell_t (* lisp_car_table[8])(const cell_t cell) =
{
  [T_NIL          ] = lisp_car_noop,
  [T_LIST         ] = lisp_car_list,
  [T_NUMBER       ] = lisp_car_noop,
  [T_STRING       ] = lisp_car_noop,
  [T_SYMBOL       ] = lisp_car_noop,
  [T_SYMBOL_INLINE] = lisp_car_noop,
  [T_TRUE         ] = lisp_car_noop,
  [T_WILDCARD     ] = lisp_car_noop,
};

cell_t
lisp_car(const cell_t cell)
{
  return lisp_car_table[GET_TYPE(cell->car)](cell);
}

static cell_t
lisp_cdr_list(const cell_t cell)
{
  cell_t L = GET_PNTR(cell_t, cell->car);
  return lisp_make_slot(L->cdr);
}

static cell_t
lisp_cdr_noop(const cell_t cell)
{
  return lisp_make_nil();
}

static cell_t (* lisp_cdr_table[8])(const cell_t cell) =
{
  [T_NIL          ] = lisp_cdr_noop,
  [T_LIST         ] = lisp_cdr_list,
  [T_NUMBER       ] = lisp_cdr_noop,
  [T_STRING       ] = lisp_cdr_noop,
  [T_SYMBOL       ] = lisp_cdr_noop,
  [T_SYMBOL_INLINE] = lisp_cdr_noop,
  [T_TRUE         ] = lisp_cdr_noop,
  [T_WILDCARD     ] = lisp_cdr_noop,
};


cell_t
lisp_cdr(const cell_t cell)
{
  return lisp_cdr_table[GET_TYPE(cell->car)](cell);
}

/*
 * Internal list construction functions.
 */

static bool
slot_equl_noop(const uintptr_t a, const uintptr_t b)
{
  return true;
}

static bool
slot_equl_number(const uintptr_t a, const uintptr_t b)
{
  return GET_NUMB(a) == GET_NUMB(b);
}

static bool
slot_equl_string(const uintptr_t a, const uintptr_t b)
{
  return strcmp(GET_PNTR(char *, a), GET_PNTR(char *, b)) == 0;
}

static bool
slot_equl_symbol(const uintptr_t a, const uintptr_t b)
{
  return strcmp(GET_SYMB(a), GET_SYMB(b)) == 0;
}

static bool
slot_equl_list(const uintptr_t a, const uintptr_t b)
{
  return lisp_equl(GET_PNTR(cell_t, a), GET_PNTR(cell_t, b));
}

static bool (* slot_equl_table[8])(const uintptr_t a, const uintptr_t b) =
{
  [T_NIL          ] = slot_equl_noop,
  [T_LIST         ] = slot_equl_list,
  [T_NUMBER       ] = slot_equl_number,
  [T_STRING       ] = slot_equl_string,
  [T_SYMBOL       ] = slot_equl_string,
  [T_SYMBOL_INLINE] = slot_equl_symbol,
  [T_TRUE         ] = slot_equl_noop,
  [T_WILDCARD     ] = slot_equl_noop,
};

static bool
slot_equl(const uintptr_t a, const uintptr_t b)
{
  return GET_TYPE(a) == GET_TYPE(b) && slot_equl_table[GET_TYPE(a)](a, b);
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
  LISP_FREE(n);
  return result;
}

static cell_t
lisp_conc_free(const cell_t a, const cell_t b)
{
  LISP_FREE(a);
  return b;
}

static cell_t
lisp_conc_list(const cell_t a, const cell_t b)
{
  FOREACH(a, p) NEXT(p);
  slot_free(p->cdr);
  p->cdr = b->car;
  lisp_deallocate(b);
  return a;
}

static cell_t (* lisp_conc_table[8])(const cell_t a, const cell_t b) =
{
  [T_NIL          ] = lisp_conc_free,
  [T_LIST         ] = lisp_conc_list,
  [T_NUMBER       ] = lisp_conc_free,
  [T_STRING       ] = lisp_conc_free,
  [T_SYMBOL       ] = lisp_conc_free,
  [T_SYMBOL_INLINE] = lisp_conc_free,
  [T_TRUE         ] = lisp_conc_free,
  [T_WILDCARD     ] = lisp_conc_free,
};

cell_t
lisp_conc(const cell_t a, const cell_t b)
{
  return lisp_conc_table[GET_TYPE(a->car)](a, b);
}

cell_t
lisp_setq(const cell_t closure, const cell_t sym, const cell_t val)
{
  cell_t con = lisp_cons(sym, val);
  /*
   * Check if the symbol exists and replace it using a zero-copy scan.
   */
  FOREACH(closure, p) {
    cell_t car = GET_PNTR(cell_t, p->car);
    if (strcmp(lisp_get_sym(car), lisp_get_sym(sym)) == 0) {
      lisp_replace(p, con);
      return closure;
    }
    NEXT(p);
  }
  /*
   * The symbol does not exist, so append it.
   */
  cell_t lst = lisp_make_list(con);
  LISP_FREE(con);
  return lisp_conc(closure, lst);
}

/*
 * Argument bindings.
 */

static cell_t
lisp_bind_noop(const cell_t closure, const cell_t args, const cell_t vals)
{
  return closure;
}

static cell_t
lisp_bind_setq(const cell_t closure, const cell_t args, const cell_t vals)
{
  return lisp_setq(closure, args, vals);
}

static cell_t
lisp_bind_list(const cell_t closure, const cell_t args, const cell_t vals)
{
  TRACE_SEXP(closure);
  cell_t sym = lisp_car(args);
  cell_t val = lisp_eval(closure, lisp_car(vals));
  cell_t oth = lisp_cdr(args);
  cell_t rem = lisp_cdr(vals);
  cell_t cl0 = lisp_bind(closure, sym, val);
  cell_t cl1 = lisp_bind(cl0, oth, rem);
  LISP_FREE(rem, oth, val, sym);
  return cl1;
}

static cell_t
lisp_bind_free(const cell_t closure, const cell_t args, const cell_t vals)
{
  LISP_FREE(closure);
  return lisp_make_nil();
}

static cell_t (* lisp_bind_table[8])(const cell_t closure, const cell_t args,
                                     const cell_t vals) =
{
  [T_NIL          ] = lisp_bind_noop,
  [T_LIST         ] = lisp_bind_list,
  [T_NUMBER       ] = lisp_bind_free,
  [T_STRING       ] = lisp_bind_free,
  [T_SYMBOL       ] = lisp_bind_setq,
  [T_SYMBOL_INLINE] = lisp_bind_setq,
  [T_TRUE         ] = lisp_bind_free,
  [T_WILDCARD     ] = lisp_bind_noop,
};

cell_t
lisp_bind(const cell_t closure, const cell_t args, const cell_t vals)
{
  cell_t res = lisp_bind_table[GET_TYPE(args->car)](closure, args, vals);
  TRACE_SEXP(res);
  return res;
}

/*
 * Body evaluation.
 */

static cell_t
lisp_prog_list(const cell_t closure, const cell_t cell, const cell_t result)
{
  TRACE_SEXP(cell);
  /*
   * Get CAR/CDR.
   */
  cell_t res = lisp_eval(closure, lisp_car(cell));
  cell_t cdr = lisp_cdr(cell);
  /*
   */
  LISP_FREE(result, cell);
  return lisp_prog(closure, cdr, res);
}

static cell_t
lisp_prog_noop(const cell_t closure, const cell_t cell, const cell_t result)
{
  TRACE_SEXP(cell);
  LISP_FREE(cell);
  return result;
}

static cell_t (* lisp_prog_table[8])(const cell_t closure, const cell_t cell,
                                     const cell_t result) =
{
  [T_NIL          ] = lisp_prog_noop,
  [T_LIST         ] = lisp_prog_list,
  [T_NUMBER       ] = lisp_prog_noop,
  [T_STRING       ] = lisp_prog_noop,
  [T_SYMBOL       ] = lisp_prog_noop,
  [T_SYMBOL_INLINE] = lisp_prog_noop,
  [T_TRUE         ] = lisp_prog_noop,
  [T_WILDCARD     ] = lisp_prog_noop,
};

cell_t
lisp_prog(const cell_t closure, const cell_t cell, const cell_t result)
{
  TRACE_SEXP(cell);
  return lisp_prog_table[GET_TYPE(cell->car)](closure, cell, result);
}

/*
 * List evaluation.
 */

static cell_t
lisp_eval_list_noop(const cell_t closure, const cell_t car, const cell_t cdr)
{
  TRACE_SEXP(car);
  TRACE_SEXP(cdr);
  cell_t cell = lisp_cons(car, cdr);
  LISP_FREE(car, cdr);
  return cell;
}

static cell_t
lisp_eval_list_number(const cell_t closure, const cell_t car, const cell_t cdr)
{
  TRACE_SEXP(car);
  TRACE_SEXP(cdr);
  uintptr_t fun = GET_NUMB(car->car);
  cell_t res = GET_PNTR(function_t, fun)(closure, cdr);
  LISP_FREE(car);
  return res;
}

static cell_t
lisp_eval_list_lambda(const cell_t closure, const cell_t lbda, const cell_t vals)
{
  TRACE_SEXP(lbda);
  TRACE_SEXP(vals);
  /*
   * Grab the arguments, body of the lambda. TODO add the curried closure.
   */
  cell_t args = lisp_car(lbda);
  cell_t body = lisp_cdr(lbda);
  /*
   * Bind the arguments and the values. TODO bind the curried closure:
   * 1. Bind arguments with values
   * 2. Check if there are any reminders
   * 3. Return a lambda with updated local closure if some are NIL
   */
  cell_t clos = lisp_bind(lisp_dup(closure), args, vals);
  cell_t rslt = lisp_prog(clos, body, lisp_make_nil());
  LISP_FREE(clos, args, lbda, vals);
  return rslt;
}

static cell_t
lisp_eval_list_symbol(const cell_t closure, const cell_t car, const cell_t cdr)
{
  TRACE_SEXP(car);
  TRACE_SEXP(cdr);
  cell_t cell = lisp_eval(closure, lisp_cons(car, cdr));
  LISP_FREE(car, cdr);
  return cell;
}

static cell_t (* lisp_eval_list_table[8])(const cell_t closure, const cell_t car,
                                          const cell_t cdr) =
{
  [T_NIL          ] = lisp_eval_list_number,
  [T_LIST         ] = lisp_eval_list_lambda,
  [T_NUMBER       ] = lisp_eval_list_number,
  [T_STRING       ] = lisp_eval_list_noop,
  [T_SYMBOL       ] = lisp_eval_list_symbol,
  [T_SYMBOL_INLINE] = lisp_eval_list_symbol,
  [T_TRUE         ] = lisp_eval_list_noop,
  [T_WILDCARD     ] = lisp_eval_list_noop,
};

/*
 * Generic evaluation.
 */

static cell_t
lisp_eval_noop(const cell_t closure, const cell_t cell)
{
  TRACE_SEXP(cell);
  return cell;
}

static cell_t
lisp_eval_symbol(const cell_t closure, const cell_t cell)
{
  TRACE_SEXP(cell);
  cell_t res = lisp_lookup(closure, cell);
  LISP_FREE(cell);
  return res;
}

static cell_t
lisp_eval_list(const cell_t closure, const cell_t cell)
{
  TRACE_SEXP(cell);
  /*
   * Evaluate CAR.
   */
  cell_t car = lisp_eval(closure, lisp_car(cell));
  cell_t cdr = lisp_cdr(cell);
  /*
   * Evaluate the list.
   */
  cell_t res = lisp_eval_list_table[GET_TYPE(car->car)](closure, car, cdr);
  LISP_FREE(cell);
  return res;
}

static cell_t (* lisp_eval_table[8])(const cell_t closure, const cell_t cell) =
{
  [T_NIL          ] = lisp_eval_noop,
  [T_LIST         ] = lisp_eval_list,
  [T_NUMBER       ] = lisp_eval_noop,
  [T_STRING       ] = lisp_eval_noop,
  [T_SYMBOL       ] = lisp_eval_symbol,
  [T_SYMBOL_INLINE] = lisp_eval_symbol,
  [T_TRUE         ] = lisp_eval_noop,
  [T_WILDCARD     ] = lisp_eval_noop,
};

cell_t
lisp_eval(const cell_t closure, const cell_t cell)
{
  TRACE_SEXP(cell);
  return lisp_eval_table[GET_TYPE(cell->car)](closure, cell);
}

/*
 * Helper functions.
 */

cell_t
lisp_make_nil()
{
  cell_t R = lisp_allocate();
  return R;
}

cell_t
lisp_make_true()
{
  cell_t R = lisp_allocate();
  SET_TYPE(R->car, T_TRUE);
  return R;
}

cell_t
lisp_make_wildcard()
{
  cell_t R = lisp_allocate();
  SET_TYPE(R->car, T_WILDCARD);
  return R;
}

cell_t
lisp_make_number(const int64_t num)
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
