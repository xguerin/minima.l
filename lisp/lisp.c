#include "lisp.h"
#include "plugin.h"
#include "slab.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Syntax error handler.
 */

error_handler_t syntax_error_handler = NULL;

void syntax_error()
{
  if (syntax_error_handler != NULL) {
    syntax_error_handler();
  }
}

void
lisp_set_syntax_error_handler(const error_handler_t handler)
{
  syntax_error_handler = handler;
}

/*
 * Symbol management.
 */

atom_t GLOBALS  = NULL;
atom_t NIL      = NULL;
atom_t TRUE     = NULL;
atom_t WILDCARD = NULL;

static atom_t
lisp_lookup(const atom_t closure, const atom_t sym)
{
  TRACE_SEXP(sym);
  /*
   * Look-up in the closure.
   */
  FOREACH(closure, a) {
    atom_t car = a->car;
    if (lisp_symbol_match(car->pair.car, sym)) {
      return UP(car->pair.cdr);
    }
    NEXT(a);
  }
  /*
   * Look-up in globals.
   */
  FOREACH(GLOBALS, b) {
    atom_t car = b->car;
    if (lisp_symbol_match(car->pair.car, sym)) {
      return UP(car->pair.cdr);
    }
    NEXT(b);
  }
  /*
   * Nothing found, try to load as a plugin.
   */
  return lisp_plugin_load(sym);
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
    case T_CHAR:
      res = lisp_make_char(atom->number);
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
      res = lisp_make_symbol(&atom->symbol);
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
    if (lisp_symbol_match(car->pair.car, sym)) {
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
  atom_t res = lisp_cons(con, closure);
  X(con); X(closure);
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
  TRACE_SEXP(closure);
  TRACE_SEXP(args);
  TRACE_SEXP(vals);
  /*
   */
  switch (args->type) {
    case T_NIL:
    case T_TRUE:
    case T_CHAR:
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
    case T_SYMBOL: {
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
    case T_CHAR:
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
      X(cell);
      /*
       * Grab the arguments, body of the lambda. TODO add the curried closure.
       */
      atom_t args = lisp_car(lbda);
      atom_t cdr0 = lisp_cdr(lbda);
      X(lbda);
      atom_t body = lisp_car(cdr0);
      X(cdr0);
      /*
       * Bind the arguments and the values. TODO bind the curried closure:
       * 1. Bind arguments with values
       * 2. Check if there are any reminders
       * 3. Return a lambda with updated local closure if some are NIL
       */
      atom_t newl = lisp_dup(closure);
      atom_t newc = lisp_bind(newl, args, vals);
      ret = lisp_eval(newc, body);
      /*
      */
      X(newc);
      break;
    }
    case T_SYMBOL: {
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
    case T_CHAR:
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
    case T_SYMBOL: {
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
lisp_make_char(const char c)
{
  atom_t R = lisp_allocate();
  R->type = T_CHAR;
  R->refs = 1;
  R->number = c;
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
lisp_make_symbol(const symbol_t sym)
{
  atom_t R = lisp_allocate();
  R->type = T_SYMBOL;
  R->refs = 1;
  R->symbol = *sym;
  TRACE_SEXP(R);
  return R;
}
