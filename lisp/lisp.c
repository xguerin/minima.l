#include "lexer.h"
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

error_handler_t lisp_parse_error_handler = NULL;
error_handler_t lisp_syntax_error_handler = NULL;

void parse_error()
{
  if (lisp_parse_error_handler != NULL) {
    lisp_parse_error_handler();
  }
}

void syntax_error()
{
  if (lisp_syntax_error_handler != NULL) {
    lisp_syntax_error_handler();
  }
}

void
lisp_set_parse_error_handler(const error_handler_t h)
{
  lisp_parse_error_handler = h;
}

void
lisp_set_syntax_error_handler(const error_handler_t h)
{
  lisp_syntax_error_handler = h;
}

/*
 * Global symbols.
 */

atom_t GLOBALS  = NULL;
atom_t ICHAN    = NULL;
atom_t OCHAN    = NULL;
atom_t NIL      = NULL;
atom_t TRUE     = NULL;
atom_t WILDCARD = NULL;

/*
 * Interpreter life cycle.
 */

static lexer_t LEXER = NULL;

static void
lisp_consumer(const atom_t cell)
{
  atom_t chn = CAR(ICHAN);
  atom_t new = lisp_cons(cell, NIL);
  atom_t old = CDR(chn);
  CDR(chn) = lisp_conc(old, new);
  X(old); X(new); X(cell);
  TRACE_SEXP(ICHAN);
}

void
lisp_init()
{
  lisp_slab_allocate();
  /*
   * Create the constants.
   */
  lisp_make_nil();
  lisp_make_true();
  lisp_make_wildcard();
  /*
   * Create the GLOBALS.
   */
  GLOBALS = UP(NIL);
  ICHAN = UP(NIL);
  OCHAN = UP(NIL);
  /*
   * Create the lexer.
   */
  LEXER = lisp_create(lisp_consumer);
}

void
lisp_fini()
{
  lisp_destroy(LEXER);
  LEXER = NULL;
  X(OCHAN); X(ICHAN); X(GLOBALS); X(WILDCARD); X(TRUE); X(NIL);
  TRACE("D %ld", slab.n_alloc - slab.n_free);
  LISP_COLLECT();
  lisp_slab_destroy();
}

/*
 * Symbol management.
 */

atom_t
lisp_lookup(const atom_t closure, const atom_t sym)
{
  TRACE_SEXP(sym);
  /*
   * Look-up in the closure.
   */
  FOREACH(closure, a) {
    atom_t car = a->car;
    if (lisp_symbol_match(CAR(car), sym)) {
      return UP(CDR(car));
    }
    NEXT(a);
  }
  /*
   * Look-up in globals.
   */
  FOREACH(GLOBALS, b) {
    atom_t car = b->car;
    if (lisp_symbol_match(CAR(car), sym)) {
      return UP(CDR(car));
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
      atom_t car = lisp_dup(CAR(atom));
      atom_t cdr = lisp_dup(CDR(atom));
      res = lisp_cons(car, cdr);
      X(car); X(cdr);
      break;
    }
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
    return UP(CAR(cell));
  }
  return UP(NIL);
}

atom_t
lisp_cdr(const atom_t cell)
{
  if (likely(IS_PAIR(cell))) {
    return UP(CDR(cell));
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
  CAR(R) = UP(car);
  CDR(R) = UP(cdr);
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
 * Lisp READ.
 */

#define RBUFLEN 1024

static atom_t
lisp_read_pop()
{
  /*
   * ((CHN0 V1 V2) (CHN1 V1 V2) ...).
   */
  atom_t chn = CAR(ICHAN);
  atom_t vls = CDR(chn);
  atom_t res = UP(CAR(vls));
  CDR(chn) = UP(CDR(vls));
  X(vls);
  TRACE_SEXP(ICHAN);
  TRACE_SEXP(res);
  return res;
}

atom_t
lisp_read(const atom_t closure, const atom_t cell)
{
  TRACE_SEXP(ICHAN);
  /*
   */
  atom_t chn = CAR(ICHAN);
  X(cell);
  /*
   * Check if there is any value in the channel's buffer.
   */
  if (CDR(chn) != NIL) {
    return lisp_read_pop();
  }
  /*
   * Read from the file descriptor.
   */
  int fd = CAR(chn)->number;
  /*
   */
  do {
    char buffer[RBUFLEN] = { 0 };
    ssize_t len = read(fd, buffer, RBUFLEN);
    if (len <= 0) break;
    lisp_parse(LEXER, buffer, len);
  }
  while (CDR(CAR(ICHAN)) == NIL || LEXER->depth != 0);
  /*
   * Grab the result and return it.
   */
  atom_t res = CDR(chn) != NIL ? lisp_read_pop(): UP(NIL);
  return res;
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
    if (lisp_symbol_match(CAR(car), sym)) {
      X(sym); X(CDR(car));
      CDR(car) = val;
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
 * PROG.
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
 * Argument bindings. RC rules: all arguments are consumed.
 */

atom_t
lisp_bind(const atom_t closure, const atom_t arg, const atom_t val)
{
  atom_t ret;
  TRACE_SEXP(closure);
  TRACE_SEXP(arg);
  TRACE_SEXP(val);
  /*
   */
  switch (arg->type) {
    case T_NIL:
    case T_TRUE:
    case T_CHAR:
    case T_NUMBER:
    case T_WILDCARD: {
      X(arg); X(val);
      ret = closure;
      break;
    }
    case T_PAIR: {
      /*
       * Grab the CARs, evaluate the value and bind them.
       */
      atom_t sym = lisp_car(arg);
      atom_t vl0 = lisp_car(val);
      atom_t cl0 = lisp_bind(closure, sym, vl0);
      /*
       * Grab the CDRs and recursively bind them.
       */
      atom_t oth = lisp_cdr(arg);
      atom_t rem = lisp_cdr(val);
      X(arg); X(val);
      /*
      */
      ret = lisp_bind(cl0, oth, rem);
      break;
    }
    case T_SYMBOL: {
      ret = lisp_setq(closure, arg, val);
      break;
    }
  }
  /*
   */
  TRACE_SEXP(ret);
  return ret;
}

static atom_t
lisp_bind_all(const atom_t closure, const atom_t args, const atom_t vals)
{
  if (args == NIL) {
    X(args); X(vals);
    return closure;
  }
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
  return lisp_bind_all(cl0, oth, rem);
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
  switch (CAR(cell)->type) {
    case T_NIL:
    case T_NUMBER: {
      atom_t cdr = lisp_cdr(cell);
      function_t fun = (function_t)CAR(cell)->number;
      X(cell);
      ret = fun(closure, cdr);
      break;
    }
    case T_TRUE:
    case T_CHAR:
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
      atom_t body = lisp_cdr(lbda);
      X(lbda);
      /*
       * Bind the arguments and the values. TODO bind the curried closure:
       * 1. Bind arguments with values
       * 2. Check if there are any reminders
       * 3. Return a lambda with updated local closure if some are NIL
       */
      atom_t newl = lisp_dup(closure);
      atom_t newc = lisp_bind_all(newl, args, vals);
      ret = lisp_prog(newc, body, UP(NIL));
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
 * Print function.
 */

static void
lisp_prin_atom(const int fd, const atom_t closure, const atom_t cell)
{
  /*
   */
  switch (cell->type) {
    case T_NIL:
      write(fd, "NIL", 3);
      break;
    case T_TRUE:
      write(fd, "T", 1);
      break;
    case T_CHAR: {
      const char c = (char)cell->number;
      write(fd, &c, 1);
      break;
    }
    case T_PAIR: {
      lisp_prin_atom(fd, closure, CAR(cell));
      if (!IS_NULL(CDR(cell))) {
        lisp_prin_atom(fd, closure, CDR(cell));
      }
      break;
    case T_NUMBER: {
      char buffer[24] = { 0 };
#ifdef __MACH__
      sprintf(buffer, "%lld", cell->number);
#else
      sprintf(buffer, "%ld", cell->number);
#endif
      write(fd, buffer, strlen(buffer));
      break;
    }
    case T_SYMBOL:
      write(fd, cell->symbol.val, strnlen(cell->symbol.val, 16));
      break;
    }
    case T_WILDCARD:
      write(fd, "_", 1);
      break;
  }
}

void
lisp_prin(const atom_t closure, const atom_t cell)
{
  int fd = CAR(CAR(OCHAN))->number;
  return lisp_prin_atom(fd, closure, cell);
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
lisp_make_symbol(const symbol_t sym)
{
  atom_t R = lisp_allocate();
  R->type = T_SYMBOL;
  R->refs = 1;
  R->symbol = *sym;
  TRACE_SEXP(R);
  return R;
}
