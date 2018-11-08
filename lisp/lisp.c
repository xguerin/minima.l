#include "functions.h"
#include "lisp.h"
#include "slab.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/*
 * Symbol management.
 */

atom_t GLOBALS = NULL;
atom_t NIL = NULL;
atom_t TRUE = NULL;
atom_t WILDCARD = NULL;

static bool
lisp_lookup_match(const atom_t a, const atom_t b)
{
  if (likely(a->type == T_INLINE && b->type == T_INLINE))
  {
    return a->tag == b->tag;
  }
  if (a->type == T_INLINE && b->type == T_SYMBOL)
  {
    return strcmp(a->symbol, b->string) == 0;
  }
  if (a->type == T_SYMBOL && b->type == T_INLINE)
  {
    return strcmp(a->string, b->symbol) == 0;
  }
  /*
   * Default case, SYM/SYM.
   */
  return strcmp(a->string, b->string) == 0;
}

static atom_t
lisp_lookup(const atom_t closure, const atom_t sym)
{
  /*
   * Look-up in the closure.
   */
  FOREACH(closure, a) {
    atom_t car = a->car;
    if (lisp_lookup_match(car->pair.car, sym)) {
      return UP(car->pair.cdr);
    }
    NEXT(a);
  }
  /*
   * Look-up in globals.
   */
  FOREACH(GLOBALS, b) {
    atom_t car = b->car;
    if (lisp_lookup_match(car->pair.car, sym)) {
      return UP(car->pair.cdr);
    }
    NEXT(b);
  }
  /*
   * Nothing found.
   */
  return UP(NIL);
}

/*
 * Basic functions.
 */

static atom_t
atom_dup_const(const atom_t atom)
{
  return UP(atom);
}

static atom_t
atom_dup_number(const atom_t atom)
{
  return lisp_make_number(atom->number);
}

static atom_t
atom_dup_string(const atom_t atom)
{
  return lisp_make_string(strdup(atom->string));
}

static atom_t
atom_dup_symbol(const atom_t atom)
{
  return lisp_make_symbol(strdup(atom->string));
}

static atom_t
atom_dup_inline(const atom_t atom)
{
  return lisp_make_inline(atom->tag);
}

static atom_t
atom_dup_pair(const atom_t atom)
{
  atom_t car = lisp_dup(atom->pair.car);
  atom_t cdr = lisp_dup(atom->pair.cdr);
  atom_t res = lisp_cons(car, cdr);
  LISP_FREE(car, cdr);
  return res;
}

static atom_t (* atom_dup_table[ATOM_TYPES])(const atom_t atom) =
{
  [T_NIL     ] = atom_dup_const,
  [T_TRUE    ] = atom_dup_const,
  [T_NUMBER  ] = atom_dup_number,
  [T_PAIR    ] = atom_dup_pair,
  [T_STRING  ] = atom_dup_string,
  [T_SYMBOL  ] = atom_dup_symbol,
  [T_INLINE  ] = atom_dup_inline,
  [T_WILDCARD] = atom_dup_const,
};

atom_t
lisp_dup(const atom_t atom)
{
  atom_t res = atom_dup_table[atom->type](atom);
  TRACE_SEXP(res);
  return res;
}

static atom_t
lisp_car_atom(const atom_t cell)
{
  return UP(NIL);
}

static atom_t
lisp_car_pair(const atom_t cell)
{
  return UP(cell->pair.car);
}

static atom_t (* lisp_car_table[ATOM_TYPES])(const atom_t cell) =
{
  [T_NIL     ] = lisp_car_atom,
  [T_TRUE    ] = lisp_car_atom,
  [T_NUMBER  ] = lisp_car_atom,
  [T_PAIR    ] = lisp_car_pair,
  [T_STRING  ] = lisp_car_atom,
  [T_SYMBOL  ] = lisp_car_atom,
  [T_INLINE  ] = lisp_car_atom,
  [T_WILDCARD] = lisp_car_atom,
};

atom_t
lisp_car(const atom_t cell)
{
  return lisp_car_table[cell->type](cell);
}

static atom_t
lisp_cdr_atom(const atom_t cell)
{
  return UP(NIL);
}

static atom_t
lisp_cdr_pair(const atom_t cell)
{
  return UP(cell->pair.cdr);
}

static atom_t (* lisp_cdr_table[ATOM_TYPES])(const atom_t cell) =
{
  [T_NIL     ] = lisp_cdr_atom,
  [T_TRUE    ] = lisp_cdr_atom,
  [T_NUMBER  ] = lisp_cdr_atom,
  [T_PAIR    ] = lisp_cdr_pair,
  [T_STRING  ] = lisp_cdr_atom,
  [T_SYMBOL  ] = lisp_cdr_atom,
  [T_INLINE  ] = lisp_cdr_atom,
  [T_WILDCARD] = lisp_cdr_atom,
};

atom_t
lisp_cdr(const atom_t cell)
{
  return lisp_cdr_table[cell->type](cell);
}

/*
 * Internal list construction functions.
 */

static bool
atom_equl_noop(const atom_t a, const atom_t b)
{
  return true;
}

static bool
atom_equl_number(const atom_t a, const atom_t b)
{
  return a->number == b->number;
}

static bool
atom_equl_string(const atom_t a, const atom_t b)
{
  return strcmp(a->string, b->string) == 0;
}

static bool
atom_equl_inline(const atom_t a, const atom_t b)
{
  return a->tag == b->tag;
}

static bool
atom_equl_pair(const atom_t a, const atom_t b)
{
  return lisp_equl(a->pair.car, b->pair.car) &&
    lisp_equl(a->pair.car, b->pair.car);
}

static bool (* atom_equl_table[8])(const atom_t a, const atom_t b) =
{
  [T_NIL     ] = atom_equl_noop,
  [T_TRUE    ] = atom_equl_noop,
  [T_NUMBER  ] = atom_equl_number,
  [T_PAIR    ] = atom_equl_pair,
  [T_STRING  ] = atom_equl_string,
  [T_SYMBOL  ] = atom_equl_string,
  [T_INLINE  ] = atom_equl_inline,
  [T_WILDCARD] = atom_equl_noop,
};

bool
lisp_equl(const atom_t a, const atom_t b)
{
  return a->type == b->type && atom_equl_table[a->type](a, b);
}

atom_t
lisp_cons(const atom_t car, const atom_t cdr)
{
  atom_t R = lisp_allocate();
  R->type = T_PAIR;
  R->refs = 1;
  R->pair.car = UP(car);
  R->pair.cdr = UP(cdr);
  TRACE_SEXP(R);
  return R;
}

static atom_t
lisp_conc_atom(const atom_t car, const atom_t cdr)
{
  atom_t R = UP(cdr);
  TRACE_SEXP(R);
  return R;
}

static atom_t
lisp_conc_pair(const atom_t car, const atom_t cdr)
{
  FOREACH(car, p) NEXT(p);
  LISP_FREE(p->cdr);
  p->cdr = UP(cdr);
  TRACE_SEXP(car);
  return UP(car);
}

static atom_t (* lisp_conc_table[ATOM_TYPES])(const atom_t a, const atom_t b) =
{
  [T_NIL     ] = lisp_conc_atom,
  [T_TRUE    ] = lisp_conc_atom,
  [T_NUMBER  ] = lisp_conc_atom,
  [T_PAIR    ] = lisp_conc_pair,
  [T_STRING  ] = lisp_conc_atom,
  [T_SYMBOL  ] = lisp_conc_atom,
  [T_INLINE  ] = lisp_conc_atom,
  [T_WILDCARD] = lisp_conc_atom,
};

atom_t
lisp_conc(const atom_t car, const atom_t cdr)
{
  return lisp_conc_table[car->type](car, cdr);
}

/*
 * SETQ. Arguments closure, sym, and vals are consumed.
 */

atom_t
lisp_setq(const atom_t closure, const atom_t sym, const atom_t val)
{
  TRACE_SEXP(closure);
  /*
   * Check if the symbol exists and replace it using a zero-copy scan.
   */
  FOREACH(closure, a) {
    atom_t car = a->car;
    if (lisp_lookup_match(car->pair.car, sym)) {
      LISP_FREE(sym, car->pair.cdr);
      car->pair.cdr = val;
      return closure;
    }
    NEXT(a);
  }
  /*
   * The symbol does not exist, so append it.
   */
  atom_t con = lisp_cons(sym, val);
  atom_t lst = lisp_cons(con, NIL);
  atom_t res = lisp_conc(closure, lst);
  LISP_FREE(closure, sym, val, lst, con);
  TRACE_SEXP(res);
  return res;
}

/*
 * Argument bindings. RC rules: all arguments are consumed.
 */

typedef atom_t (* lisp_binder_t)(const atom_t, const atom_t, const atom_t);

static atom_t
lisp_bind_noop(const atom_t closure, const atom_t args, const atom_t vals)
{
  LISP_FREE(args, vals);
  return closure;
}

static atom_t
lisp_bind_setq(const atom_t closure, const atom_t args, const atom_t vals)
{
  TRACE_SEXP(closure);
  atom_t ret = lisp_setq(closure, args, vals);
  TRACE_SEXP(closure);
  TRACE_SEXP(ret);
  return ret;
}

static atom_t
lisp_bind_pair(const atom_t closure, const atom_t args, const atom_t vals)
{
  TRACE_SEXP(closure);
  /*
   * Grab the CARs, evaluate the value and bind them.
   */
  atom_t sym = lisp_car(args);
  atom_t val = lisp_eval(closure, lisp_car(vals));
  atom_t cl0 = lisp_bind(closure, sym, val);
  TRACE_SEXP(closure);
  TRACE_SEXP(cl0);
  /*
   * Grab the CDRs and recursively bind them.
   */
  atom_t oth = lisp_cdr(args);
  atom_t rem = lisp_cdr(vals);
  atom_t res = lisp_bind(cl0, oth, rem);
  TRACE_SEXP(cl0);
  TRACE_SEXP(res);
  /*
   */
  LISP_FREE(args, vals);
  TRACE_SEXP(res);
  return res;
}

static lisp_binder_t lisp_bind_table[ATOM_TYPES] =
{
  [T_NIL     ] = lisp_bind_noop,
  [T_TRUE    ] = lisp_bind_noop,
  [T_NUMBER  ] = lisp_bind_noop,
  [T_PAIR    ] = lisp_bind_pair,
  [T_STRING  ] = lisp_bind_noop,
  [T_SYMBOL  ] = lisp_bind_setq,
  [T_INLINE  ] = lisp_bind_setq,
  [T_WILDCARD] = lisp_bind_noop,
};

atom_t
lisp_bind(const atom_t closure, const atom_t args, const atom_t vals)
{
  return lisp_bind_table[args->type](closure, args, vals);
}

/*
 * Prog evaluation. The result of the last evaluation is returned.
 */

static atom_t
lisp_prog_noop(const atom_t closure, const atom_t cell, const atom_t result)
{
  TRACE_SEXP(cell);
  LISP_FREE(cell);
  return result;
}

static atom_t
lisp_prog_list(const atom_t closure, const atom_t cell, const atom_t result)
{
  TRACE_SEXP(cell);
  /*
   * Get CAR/CDR.
   */
  atom_t res = lisp_eval(closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  /*
   */
  LISP_FREE(cell, result);
  return lisp_prog(closure, cdr, res);
}

static atom_t (* lisp_prog_table[ATOM_TYPES])(const atom_t closure,
                                              const atom_t cell,
                                              const atom_t result) =
{
  [T_NIL     ] = lisp_prog_noop,
  [T_TRUE    ] = lisp_prog_noop,
  [T_NUMBER  ] = lisp_prog_noop,
  [T_PAIR    ] = lisp_prog_list,
  [T_STRING  ] = lisp_prog_noop,
  [T_SYMBOL  ] = lisp_prog_noop,
  [T_INLINE  ] = lisp_prog_noop,
  [T_WILDCARD] = lisp_prog_noop,
};

atom_t
lisp_prog(const atom_t closure, const atom_t cell, const atom_t result)
{
  TRACE_SEXP(cell);
  return lisp_prog_table[cell->type](closure, cell, result);
}

/*
 * List evaluation.
 */

static atom_t
lisp_eval_list_noop(const atom_t closure, const atom_t cell)
{
  TRACE_SEXP(cell);
  return cell;
}

static atom_t
lisp_eval_list_number(const atom_t closure, const atom_t cell)
{
  TRACE_SEXP(cell);
  atom_t car = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  uintptr_t fun = car->number;
  LISP_FREE(car, cell);
  return ((function_t)car->number)(closure, cdr);
}

static atom_t
lisp_eval_list_lambda(const atom_t closure, const atom_t cell)
{
  TRACE_SEXP(cell);
  /*
   * Grab lambda and values.
   */
  atom_t lbda = lisp_car(cell);
  atom_t vals = lisp_cdr(cell);
  /*
   * Grab the arguments, body of the lambda. TODO add the curried closure.
   */
  atom_t args = lisp_car(lbda);
  atom_t body = lisp_cdr(lbda);
  /*
   * Bind the arguments and the values. TODO bind the curried closure:
   * 1. Bind arguments with values
   * 2. Check if there are any reminders
   * 3. Return a lambda with updated local closure if some are NIL
   */
  atom_t newl = lisp_dup(closure);
  atom_t clos = lisp_bind(newl, args, vals);
  TRACE_SEXP(clos);
  /*
   */
  atom_t rslt = lisp_prog(clos, body, UP(NIL));
  LISP_FREE(clos, cell, lbda);
  return rslt;
}

static atom_t
lisp_eval_list_symbol(const atom_t closure, const atom_t cell)
{
  TRACE_SEXP(cell);
  return lisp_eval(closure, cell);
}

static atom_t (* lisp_eval_list_table[ATOM_TYPES])(const atom_t closure,
                                                   const atom_t cell) =
{
  [T_NIL     ] = lisp_eval_list_number,
  [T_TRUE    ] = lisp_eval_list_noop,
  [T_NUMBER  ] = lisp_eval_list_number,
  [T_PAIR    ] = lisp_eval_list_lambda,
  [T_STRING  ] = lisp_eval_list_noop,
  [T_SYMBOL  ] = lisp_eval_list_symbol,
  [T_INLINE  ] = lisp_eval_list_symbol,
  [T_WILDCARD] = lisp_eval_list_noop,
};

/*
 * Generic evaluation.
 */

static atom_t
lisp_eval_noop(const atom_t closure, const atom_t cell)
{
  TRACE_SEXP(cell);
  /*
   * Equivalent to DUP + FREE.
   */
  return cell;
}

static atom_t
lisp_eval_symbol(const atom_t closure, const atom_t cell)
{
  TRACE_SEXP(cell);
  atom_t res = lisp_lookup(closure, cell);
  LISP_FREE(cell);
  return res;
}

static atom_t
lisp_eval_list(const atom_t closure, const atom_t cell)
{
  TRACE_SEXP(cell);
  /*
   * Evaluate CAR.
   */
  atom_t car = lisp_eval(closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  atom_t new = lisp_cons(car, cdr);
  /*
   * Evaluate the list.
   */
  LISP_FREE(car, cdr, cell);
  return lisp_eval_list_table[car->type](closure, new);
}

static atom_t (* lisp_eval_table[ATOM_TYPES])(const atom_t closure,
                                              const atom_t cell) =
{
  [T_NIL     ] = lisp_eval_noop,
  [T_TRUE    ] = lisp_eval_noop,
  [T_NUMBER  ] = lisp_eval_noop,
  [T_PAIR    ] = lisp_eval_list,
  [T_STRING  ] = lisp_eval_noop,
  [T_SYMBOL  ] = lisp_eval_symbol,
  [T_INLINE  ] = lisp_eval_symbol,
  [T_WILDCARD] = lisp_eval_noop,
};

atom_t
lisp_eval(const atom_t closure, const atom_t cell)
{
  TRACE_SEXP(cell);
  return lisp_eval_table[cell->type](closure, cell);
}

/*
 * Helper functions.
 */

void
lisp_make_nil()
{
  atom_t R = lisp_allocate();
  R->type = T_NIL;
  R->refs = 1;
  TRACE_SEXP(R);
  NIL = R;
}

void
lisp_make_true()
{
  atom_t R = lisp_allocate();
  R->type = T_TRUE;
  R->refs = 1;
  TRACE_SEXP(R);
  TRUE = R;
}

void
lisp_make_wildcard()
{
  atom_t R = lisp_allocate();
  R->type = T_WILDCARD;
  R->refs = 1;
  TRACE_SEXP(R);
  WILDCARD = R;
}

atom_t
lisp_make_inline(const uint64_t tag)
{
  atom_t R = lisp_allocate();
  R->type = T_INLINE;
  R->refs = 1;
  R->tag = tag;
  TRACE_SEXP(R);
  return R;
}

atom_t
lisp_make_number(const int64_t num)
{
  atom_t R = lisp_allocate();
  R->type = T_NUMBER;
  R->refs = 1;
  R->number = num;
  TRACE_SEXP(R);
  return R;
}

atom_t
lisp_make_string(const char * const str)
{
  atom_t R = lisp_allocate();
  R->type = T_STRING;
  R->refs = 1;
  R->string = str;
  TRACE_SEXP(R);
  return R;
}

atom_t
lisp_make_symbol(const char * const sym)
{
  atom_t R = lisp_allocate();
  R->type = T_SYMBOL;
  R->refs = 1;
  R->string = sym;
  TRACE_SEXP(R);
  return R;
}
