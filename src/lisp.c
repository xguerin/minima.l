#include <mnml/debug.h>
#include <mnml/lexer.h>
#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
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
atom_t PLUGINS  = NULL;
atom_t ICHAN    = NULL;
atom_t OCHAN    = NULL;
atom_t NIL      = NULL;
atom_t TRUE     = NULL;
atom_t WILDCARD = NULL;

/*
 * Interpreter life cycle.
 */

static void
lisp_consumer(const atom_t cell)
{
  atom_t chn = CAR(ICHAN);
  CDR(chn) = lisp_append(CDR(chn), cell);
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
  PLUGINS = UP(NIL);
  ICHAN   = UP(NIL);
  OCHAN   = UP(NIL);
  /*
   * Setup the debug variables.
   */
#ifdef LISP_ENABLE_DEBUG
  MNML_DEBUG        = getenv("MNML_DEBUG")        != NULL;
  MNML_VERBOSE_CONS = getenv("MNML_VERBOSE_CONS") != NULL;
  MNML_VERBOSE_RC   = getenv("MNML_VERBOSE_RC")   != NULL;
  MNML_VERBOSE_SLOT = getenv("MNML_VERBOSE_SLOT") != NULL;
  MNML_VERBOSE_SLAB = getenv("MNML_VERBOSE_SLAB") != NULL;
#endif
}

void
lisp_fini()
{
  lisp_plugin_cleanup();
  X(OCHAN); X(ICHAN); X(PLUGINS); X(GLOBALS); X(WILDCARD); X(TRUE); X(NIL);
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
  TRACE_CONS(R);
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
  TRACE_CONS(R);
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
  lexer_t lexer;
  lisp_create(lisp_consumer, &lexer);
  int fd = CAR(chn)->number;
  /*
   */
  ssize_t len = 0;
  char buffer[RBUFLEN] = { 0 };
  do {
    len = read(fd, buffer + lexer.rem, RBUFLEN - lexer.rem);
    lisp_parse(&lexer, buffer, lexer.rem + len, lexer.rem + len < RBUFLEN);
    if (len <= 0) break;
  }
  while (CDR(CAR(ICHAN)) == NIL || lisp_pending(&lexer));
  /*
   * Grab the result and return it.
   */
  lisp_destroy(&lexer);
  return CDR(chn) != NIL ? lisp_read_pop(): UP(NIL);
}

/*
 * SETQ. Arguments closure, sym, and vals are consumed.
 */

atom_t
lisp_setq(const atom_t closure, const atom_t sym, const atom_t val)
{
  /*
   * Check if the symbol exists and replace it using a zero-copy scan.
   *
   * NOTE
   *
   * When the symbol exists, replace the whole pair. We do this to support
   * shallow-copying the closure when calling a lambda (see lisp_dup).
   */
  FOREACH(closure, a) {
    atom_t car = a->car;
    if (lisp_symbol_match(CAR(car), sym)) {
      X(car);
      a->car = lisp_cons(sym, val);
      X(sym); X(val);
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
lisp_bind_args(const atom_t closure, const atom_t env, const atom_t args,
               const atom_t vals, atom_t * const rest)
{
  TRACE_SEXP(closure);
  TRACE_SEXP(env);
  TRACE_SEXP(args);
  TRACE_SEXP(vals);
  /*
   * Return if we run out of arguments or values.
   */
  if (IS_NULL(args) || IS_NULL(vals)) {
    *rest = args;
    X(vals);
    return env;
  }
  /*
   * Grab the CARs, evaluate the value and bind them.
   */
  atom_t sym = lisp_car(args);
  atom_t val = lisp_eval(closure, lisp_car(vals));
  atom_t cl0 = lisp_bind(env, sym, val);
  /*
   * Grab the CDRs and recursively bind them.
   */
  atom_t oth = lisp_cdr(args);
  atom_t rem = lisp_cdr(vals);
  X(args); X(vals);
  /*
  */
  atom_t res = lisp_bind_args(closure, cl0, oth, rem, rest);
  TRACE_SEXP(res);
  return res;
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
       * Grab the arguments, the closure and the body of the lambda.
       */
      atom_t args = lisp_car(lbda);
      atom_t cdr0 = lisp_cdr(lbda);
      X(lbda);
      atom_t lcls = lisp_car(cdr0);
      atom_t body = lisp_cdr(cdr0);
      X(cdr0);
      /*
       * Bind the arguments and the values. The closure embedded in the lambda
       * is used as the run environment and augmented with the arguments'
       * values. The call-site closure is used for the evaluation of the
       * arguments.
       */
      atom_t rest;
      atom_t newl = lisp_merge(lcls, lisp_dup(closure));
      atom_t newc = lisp_bind_args(closure, newl, args, vals, &rest);
      /*
       * Partial application:
       * 1. Have lisp_bind_all return a list of unbound arguments
       * 2. If that list is NIL, evaluate the function
       * 3. Otherwise, build a new function with the remaining arguments
       */
      if (IS_NULL(rest)) {
        ret = lisp_prog(newc, body, UP(NIL));
        X(rest); X(newc);
      }
      else {
        atom_t con = lisp_cons(newc, body);
        ret = lisp_cons(rest, con);
        X(con); X(rest); X(newc); X(body);
      }
      /*
       */
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

#define IO_BUFFER_LEN 1024

static size_t
lisp_prin_atom(const int fd, char * const buf, const size_t idx,
               const atom_t closure, const atom_t cell, const bool s);

static size_t
lisp_write(const int fd, char * const buf, const size_t idx,
           void * data, const size_t len)
{
  size_t pidx = idx;
  /*
   * Flush the buffer if necessary.
   */
  if (pidx + len >= IO_BUFFER_LEN) {
    write(fd, buf, pidx);
    pidx = 0;
  }
  /*
   * Append the new data.
   */
  memcpy(&buf[pidx], data, len);
  return pidx + len;
}

static void
lisp_flush(const int fd, char * const buf, const size_t idx)
{
  write(fd, buf, idx);
}

static size_t
lisp_prin_pair(const int fd, char * const buf, const size_t idx,
               const atom_t closure, const atom_t cell, const bool s)
{
  size_t nxt = 0;
  /*
   * Print CAR.
   */
  nxt = lisp_prin_atom(fd, buf, idx, closure, CAR(cell), s);
  /*
   * Print CDR.
   */
  if (!IS_NULL(CDR(cell))) {
    if (IS_PAIR(CDR(cell))) {
      if (s) nxt = lisp_write(fd, buf, nxt, " ", 1);
      return lisp_prin_pair(fd, buf, nxt, closure, CDR(cell), s);
    }
    else {
      if (s) nxt = lisp_write(fd, buf, nxt, " . ", 3);
      return lisp_prin_atom(fd, buf, nxt, closure, CDR(cell), s);
    }
  }
  /*
   */
  return nxt;
}

static size_t
lisp_prin_atom(const int fd, char * const buf, const size_t idx,
               const atom_t closure, const atom_t cell, const bool s)
{
  switch (cell->type) {
    case T_NIL:
      return lisp_write(fd, buf, idx, "NIL", 3);
    case T_TRUE:
      return lisp_write(fd, buf, idx, "T", 1);
    case T_CHAR: {
      char c = (char)cell->number;
      if (s) {
        size_t nxt = lisp_write(fd, buf, idx, "'", 1);
        switch (c) {
          case '\n':
            nxt = lisp_write(fd, buf, nxt, "\\n", 2);
            break;
          case '\t':
            nxt = lisp_write(fd, buf, nxt, "\\t", 2);
            break;
          default:
            nxt = lisp_write(fd, buf, nxt, &c, 1);
            break;
        }
        return lisp_write(fd, buf, nxt, "'", 1);
      }
      return lisp_write(fd, buf, idx, &c, 1);
    }
    case T_PAIR: {
      size_t nxt = idx;
      if (s) nxt = lisp_write(fd, buf, nxt, "(", 1);
      nxt = lisp_prin_pair(fd, buf, nxt, closure, cell, s);
      if (s) nxt = lisp_write(fd, buf, nxt, ")", 1);
      return nxt;
    }
    case T_NUMBER: {
      char buffer[24] = { 0 };
#ifdef __MACH__
      sprintf(buffer, "%lld", cell->number);
#else
      sprintf(buffer, "%ld", cell->number);
#endif
      return lisp_write(fd, buf, idx, buffer, strlen(buffer));
    }
    case T_SYMBOL:
      return lisp_write(fd, buf, idx, cell->symbol.val,
                        strnlen(cell->symbol.val, 16));
    case T_WILDCARD:
      return lisp_write(fd, buf, idx, "_", 1);
  }
}

void
lisp_prin(const atom_t closure, const atom_t cell, const bool s)
{
  int fd = CAR(CAR(OCHAN))->number;
  char buffer[IO_BUFFER_LEN];
  size_t idx = lisp_prin_atom(fd, buffer, 0, closure, cell, s);
  lisp_flush(fd, buffer, idx);
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
lisp_make_string(const char * const str, const size_t len)
{
  atom_t res = UP(NIL);
  for (size_t i = 0; i < len; i += 1) {
    atom_t c = lisp_make_char(str[len - i - 1]);
    atom_t n = lisp_cons(c, res);
    X(c); X(res);
    res = n;
  }
  return lisp_process_escapes(res, false, UP(NIL));
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
