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

atom_t GLOBALS = NULL;
atom_t NIL = NULL;
atom_t TRUE = NULL;
atom_t WILDCARD = NULL;

static bool
lisp_lookup_match(const atom_t a, const atom_t b)
{
  if (likely(IS_INLN(a) && IS_INLN(b)))
  {
    return a->tag == b->tag;
  }
  if (IS_INLN(a) && IS_SYMB(b))
  {
    return strcmp(a->symbol, b->string) == 0;
  }
  if (IS_SYMB(a) && IS_INLN(b))
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

atom_t
lisp_dup(const atom_t atom)
{
  atom_t res;
  /*
   */
  switch (atom->type) {
    case T_NIL:
    case T_TRUE:
    case T_WILDCARD:
      res = UP(atom);
      break;
    case T_NUMBER:
      res = lisp_make_number(atom->number);
      break;
    case T_PAIR: {
      atom_t car = lisp_dup(atom->pair.car);
      atom_t cdr = lisp_dup(atom->pair.cdr);
      res = lisp_cons(car, cdr);
      X(car); X(cdr);
      break;
    }
    case T_STRING:
      res = lisp_make_string(strdup(atom->string));
      break;
    case T_SYMBOL:
      res = lisp_make_symbol(strdup(atom->string));
      break;
    case T_INLINE:
      res = lisp_make_inline(atom->tag);
      break;
  }
  /*
   */
  TRACE_SEXP(res);
  return res;
}

atom_t
lisp_car(const atom_t cell)
{
  if (likely(IS_PAIR(cell))) {
    return UP(cell->pair.car);
  }
  return UP(NIL);
}

atom_t
lisp_cdr(const atom_t cell)
{
  if (likely(IS_PAIR(cell))) {
    return UP(cell->pair.cdr);
  }
  return UP(NIL);
}

/*
 * Internal list construction functions.
 */

static bool
atom_equl(const atom_t a, const atom_t b)
{
  switch (a->type) {
    case T_NIL:
    case T_TRUE:
    case T_WILDCARD:
      return true;
    case T_NUMBER:
      return a->number == b->number;
    case T_PAIR:
      return lisp_equl(a->pair.car, b->pair.car) &&
        lisp_equl(a->pair.car, b->pair.car);
    case T_STRING:
    case T_SYMBOL:
      return strcmp(a->string, b->string) == 0;
    case T_INLINE:
      return a->tag == b->tag;
  }
}

bool
lisp_equl(const atom_t a, const atom_t b)
{
  return a->type == b->type && atom_equl(a, b);
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

atom_t
lisp_conc(const atom_t car, const atom_t cdr)
{
  atom_t R;
  /*
   */
  if (likely(IS_PAIR(car))) {
    FOREACH(car, p) NEXT(p);
    X(p->cdr);
    p->cdr = UP(cdr);
    R = UP(car);
  }
  else {
    R = UP(cdr);
  }
  /*
   */
  TRACE_SEXP(R);
  return R;
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
      X(sym); X(car->pair.cdr);
      car->pair.cdr = val;
      return closure;
    }
    NEXT(a);
  }
  /*
   * The symbol does not exist, so append it.
   */
  atom_t con = lisp_cons(sym, val);
  X(sym); X(val);
  atom_t lst = lisp_cons(con, NIL);
  X(con);
  atom_t res = lisp_conc(closure, lst);
  X(lst); X(closure);
  /*
   */
  TRACE_SEXP(res);
  return res;
}

/*
 * Argument bindings. RC rules: all arguments are consumed.
 */

atom_t
lisp_bind(const atom_t closure, const atom_t args, const atom_t vals)
{
  atom_t ret;
  /*
   */
  switch (args->type) {
    case T_NIL:
    case T_TRUE:
    case T_NUMBER:
    case T_STRING:
    case T_WILDCARD: {
      X(args); X(vals);
      ret = closure;
      break;
    }
    case T_PAIR: {
      /*
       * Grab the CARs, evaluate the value and bind them.
       */
      atom_t sym = lisp_car(args);
      atom_t val = lisp_eval(closure, lisp_car(vals));
      atom_t cl0 = lisp_bind(closure, sym, val);
      /*
       * Grab the CDRs and recursively bind them.
       */
      atom_t oth = lisp_cdr(args);
      atom_t rem = lisp_cdr(vals);
      X(args); X(vals);
      /*
      */
      ret = lisp_bind(cl0, oth, rem);
      break;
    }
    case T_SYMBOL:
    case T_INLINE: {
      ret = lisp_setq(closure, args, vals);
      break;
    }
  }
  /*
   */
  TRACE_SEXP(ret);
  return ret;
}

/*
 * Prog evaluation. The result of the last evaluation is returned.
 */

atom_t
lisp_prog(const atom_t closure, const atom_t cell, const atom_t result)
{
  TRACE_SEXP(cell);
  /*
   */
  if (likely(IS_PAIR(cell))) {
    /*
     * Get CAR/CDR.
     */
    atom_t res = lisp_eval(closure, lisp_car(cell));
    atom_t cdr = lisp_cdr(cell);
    /*
     */
    X(cell); X(result);
    return lisp_prog(closure, cdr, res);
  }
  /*
   */
  X(cell);
  return result;
}

/*
 * Prog creation. The list of the evaluations is returned.
 */

atom_t
lisp_list(const atom_t closure, const atom_t cell)
{
  TRACE_SEXP(cell);
  /*
   */
  if (likely(IS_PAIR(cell))) {
    /*
     * Get CAR/CDR.
     */
    atom_t car = lisp_car(cell);
    atom_t cdr = lisp_cdr(cell);
    X(cell);
    /*
     * Recursively get the result.
     */
    atom_t res = lisp_list(closure, cdr);
    atom_t evl = lisp_eval(closure, car);
    atom_t con = lisp_cons(evl, res);
    X(evl); X(res);
    return con;
  }
  /*
   */
  return cell;
}

/*
 * Prog stream. The result of each evaluation are chained.
 */

atom_t
lisp_pipe(const atom_t closure, const atom_t cell, const atom_t result)
{
  TRACE_SEXP(cell);
  /*
   */
  if (likely(IS_PAIR(cell))) {
    /*
     * Get CAR/CDR.
     */
    atom_t car = lisp_car(cell);
    atom_t cdr = lisp_cdr(cell);
    X(cell);
    atom_t lsp = lisp_cons(result, NIL);
    X(result);
    atom_t env = lisp_cons(car, lsp);
    X(car); X(lsp);
    /*
     */
    atom_t res = lisp_eval(closure, env);
    return lisp_pipe(closure, cdr, res);
  }
  /*
   */
  X(cell);
  return result;
}

/*
 * List evaluation.
 */

static atom_t
lisp_eval_pair(const atom_t closure, const atom_t cell)
{
  atom_t ret;
  TRACE_SEXP(cell);
  /*
   */
  switch (cell->pair.car->type) {
    case T_NIL:
    case T_NUMBER: {
      atom_t cdr = lisp_cdr(cell);
      function_t fun = (function_t)cell->pair.car->number;
      X(cell);
      ret = fun(closure, cdr);
      break;
    }
    case T_TRUE:
    case T_STRING:
    case T_WILDCARD: {
      ret = cell;
      break;
    }
    case T_PAIR: {
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
      X(cell); X(lbda);
      /*
       * Bind the arguments and the values. TODO bind the curried closure:
       * 1. Bind arguments with values
       * 2. Check if there are any reminders
       * 3. Return a lambda with updated local closure if some are NIL
       */
      atom_t newl = lisp_dup(closure);
      atom_t clos = lisp_bind(newl, args, vals);
      ret = lisp_prog(clos, body, UP(NIL));
      /*
      */
      X(clos);
      break;
    }
    case T_SYMBOL:
    case T_INLINE: {
      ret = lisp_eval(closure, cell);
      break;
    }
  }
  /*
   */
  TRACE_SEXP(ret);
  return ret;
}

/*
 * Generic evaluation.
 */

atom_t
lisp_eval(const atom_t closure, const atom_t cell)
{
  atom_t ret;
  TRACE_SEXP(cell);
  /*
   */
  switch (cell->type) {
    case T_NIL:
    case T_TRUE:
    case T_NUMBER:
    case T_STRING:
    case T_WILDCARD: {
      ret = cell;
      break;
    }
    case T_PAIR: {
      /*
       * Evaluate CAR.
       */
      atom_t car = lisp_eval(closure, lisp_car(cell));
      atom_t cdr = lisp_cdr(cell);
      X(cell);
      /*
       * Build the new value.
       */
      atom_t new = lisp_cons(car, cdr);
      X(car); X(cdr);
      /*
       * Evaluate the list.
       */
      ret = lisp_eval_pair(closure, new);
      break;
    }
    case T_SYMBOL:
    case T_INLINE: {
      ret = lisp_lookup(closure, cell);
      X(cell);
      break;
    }
  }
  /*
   */
  TRACE_SEXP(ret);
  return ret;
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
