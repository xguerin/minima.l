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
 * Global symbols.
 */

atom_t GLOBALS  = NULL;
atom_t PLUGINS  = NULL;
atom_t ICHAN    = NULL;
atom_t OCHAN    = NULL;
atom_t NIL      = NULL;
atom_t TRUE     = NULL;
atom_t QUOTE    = NULL;
atom_t WILDCARD = NULL;

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
      TRACE("0x%lx", (uintptr_t)car);
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
      TRACE("0x%lx", (uintptr_t)car);
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

static void
lisp_consumer(const atom_t cell)
{
  atom_t chn = CAR(ICHAN);
  CDR(chn) = lisp_append(CDR(chn), cell);
  TRACE_SEXP(ICHAN);
}

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
lisp_setq(const atom_t closure, const atom_t pair)
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
    if (car != pair && lisp_symbol_match(CAR(car), CAR(pair))) {
      X(car);
      a->car = pair;
      return closure;
    }
    NEXT(a);
  }
  /*
   * The symbol does not exist, so append it.
   */
  atom_t res = lisp_cons(pair, closure);
  X(pair); X(closure);
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
 * RTRN.
 */

atom_t
lisp_rtrn(const atom_t closure, const atom_t rslt, const atom_t cont)
{
  TRACE_SEXP(rslt);
  TRACE_SEXP(cont);
  /*
   * If the continuation is NIL, return the result.
   */
  if (IS_NULL(cont)) {
    X(cont);
    return rslt;
  }
  /*
   * Otherwise, call the continuation on the result.
   */
  atom_t lst = lisp_cons(rslt, NIL);
  atom_t fun = lisp_cons(cont, lst);
  X(lst); X(cont); X(rslt);
  return lisp_eval(closure, fun);
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
      ret = lisp_setq(closure, lisp_cons(arg, val));
      X(arg); X(val);
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
               const atom_t vals, atom_t * const narg, atom_t * const nval)
{
  TRACE_SEXP(closure);
  TRACE_SEXP(env);
  TRACE_SEXP(args);
  TRACE_SEXP(vals);
  /*
   * Return if we run out of arguments or values.
   */
  if (IS_NULL(args) || IS_NULL(vals)) {
    *narg = args;
    *nval = vals;
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
  atom_t res = lisp_bind_args(closure, cl0, oth, rem, narg, nval);
  TRACE_SEXP(res);
  return res;
}

/*
 * Lambda evaluation.
 */

static atom_t
lisp_eval_lambda(const atom_t closure, const atom_t cell, atom_t * const rem)
{
  atom_t ret;
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
   * NOTE: Merge the define-site closure into the call-site closure.
   * This is required by locally recursive lambdas. Define-site definitions
   * take precendence:
   *
   * atom_t newl = lisp_merge(lcls, lisp_dup(closure));
   *
   * This is disabled for the moment as when merging both closures, we end up
   * with a lot more than what we bargained for when calling external functions.
   *
   * TODO find a better way to handle locally recursive functions. Ideally, it
   * should be handled in `let` as this is where the binding occurs.
   */
  atom_t newl = lisp_dup(lcls);
  X(lcls);
  /*
   * Bind the arguments and the values. The closure embedded in the lambda
   * is used as the run environment and augmented with the arguments'
   * values. The call-site closure is used for the evaluation of the
   * arguments.
   */
  atom_t narg;
  atom_t newc = lisp_bind_args(closure, newl, args, vals, &narg, rem);
  /*
   * If the list of remaining arguments is not NIL, handle partial
   * application.
   */
  if (!IS_NULL(narg)) {
    atom_t con = lisp_cons(newc, body);
    ret = lisp_cons(narg, con);
    X(con); X(body);
  }
  /*
   * Otherwise, evaluation the function.
   */
  else {
    ret = lisp_prog(newc, body, UP(NIL));
  }
  /*
   */
  X(narg); X(newc);
  return ret;
}

/*
 * List evaluation.
 */

static atom_t
lisp_eval_pair(const atom_t closure, const atom_t cell)
{
  atom_t ret, rem;
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
      rem = UP(NIL);
      break;
    }
    case T_TRUE:
    case T_CHAR:
    case T_WILDCARD:
      ret = cell;
      rem = UP(NIL);
      break;
    case T_PAIR:
      ret = lisp_eval_lambda(closure, cell, &rem);
      break;
    case T_SYMBOL:
      ret = lisp_eval(closure, cell);
      rem = UP(NIL);
      break;
  }
  /*
   * Check if there is any remainder arguments.
   */
  if (IS_NULL(rem)) {
    X(rem);
    TRACE_SEXP(ret);
    return ret;
  }
  /*
   * Apply the result to the remainder arguments.
   */
  atom_t old = ret;
  ret = lisp_cons(old, rem);
  X(old); X(rem);
  /*
   */
  TRACE_SEXP(ret);
  return lisp_eval_pair(closure, ret);
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
