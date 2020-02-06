#include <mnml/debug.h>
#include <mnml/lexer.h>
#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Global symbols.
 */

atom_t ICHAN    = NULL;
atom_t OCHAN    = NULL;
atom_t NIL      = NULL;
atom_t TRUE     = NULL;
atom_t QUOTE    = NULL;
atom_t WILDCARD = NULL;

/*
 * Symbol lookup.
 */

atom_t
lisp_lookup_immediate(const atom_t closure, const symbol_t sym)
{
  /*
   * Look-up in the closure.
   */
  FOREACH(closure, a) {
    atom_t car = a->car;
    if (lisp_symbol_match_immediate(CAR(car), sym)) {
      return UP(CDR(car));
    }
    NEXT(a);
  }
  /*
   * Nothing found.
   */
  return UP(NIL);
}

atom_t
lisp_lookup(const lisp_t lisp, const atom_t closure, const atom_t sym)
{
  /*
   * Default condition, check the global environemnt.
   */
  if (IS_NULL(closure)) {
    FOREACH(lisp->GLOBALS, g) {
      atom_t car = g->car;
      if (lisp_symbol_match(CAR(car), sym)) {
        return UP(CDR(car));
      }
      NEXT(g);
    }
    /*
     * Nothing found.
     */
    return UP(NIL);
  }
  /*
   * Look for the symbol up the closure stack.
   */
  FOREACH(CAR(closure), a) {
    atom_t car = a->car;
    if (lisp_symbol_match(CAR(car), sym)) {
      return UP(CDR(car));
    }
    NEXT(a);
  }
  /*
   * Check the next level.
   */
  return lisp_lookup(lisp, CDR(closure), sym);
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
  TRACE_CONS_SEXP(R);
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
  TRACE_CONS_SEXP(R);
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
}

static atom_t
lisp_read_pop()
{
  TRACE_CHAN_SEXP(ICHAN);
  /*
   * ((CHN0 PWD V1 V2) (CHN1 PWD V1 V2) ...).
   */
  atom_t chn = CAR(ICHAN);
  atom_t vls = CDR(CDR(chn));
  atom_t res = UP(CAR(vls));
  CDR(CDR(chn)) = UP(CDR(vls));
  X(vls);
  /*
   */
  TRACE_CHAN_SEXP(ICHAN);
  return res;
}

atom_t
lisp_read(const lisp_t lisp, const atom_t closure, const atom_t cell)
{
  TRACE_CHAN_SEXP(ICHAN);
  X(cell);
  /*
   * Grab the channel, the path and the content.
   */
  atom_t chn = CAR(ICHAN);
  atom_t hnd = CAR(chn);
  atom_t val = CDR(CDR(chn));
  /*
   * Check if there is any value in the channel's buffer.
   */
  if (!IS_NULL(val)) {
    return lisp_read_pop();
  }
  /*
   * Read from the file descriptor.
   */
  lexer_t lexer;
  lisp_lexer_create(lisp, lisp_consumer, &lexer);
  FILE* handle = (FILE *)hnd->number;
  /*
   */
  char buffer[RBUFLEN] = { 0 };
  do {
    char* p = fgets(buffer + lexer.rem, RBUFLEN - lexer.rem, handle);
    if (p == NULL) {
      break;
    }
    size_t len = strlen(p);
    lisp_lexer_parse(&lexer, buffer, lexer.rem + len, lexer.rem + len < RBUFLEN);
  }
  while (CDR(CDR(CAR(ICHAN))) == NIL || lisp_lexer_pending(&lexer));
  /*
   * Grab the result and return it.
   */
  lisp_lexer_destroy(&lexer);
  return CDR(CDR(chn)) != NIL ? lisp_read_pop(): NULL;
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
  X(pair, closure);
  /*
   */
  return res;
}

/*
 * PROG.
 */

atom_t
lisp_prog(const lisp_t lisp, const atom_t closure, const atom_t cell,
          const atom_t result)
{
  if (likely(IS_PAIR(cell))) {
    /*
     * Get CAR/CDR.
     */
    atom_t res = lisp_eval(lisp, closure, lisp_car(cell));
    atom_t cdr = lisp_cdr(cell);
    /*
     */
    X(cell, result);
    return lisp_prog(lisp, closure, cdr, res);
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
lisp_bind(const lisp_t lisp, const atom_t closure, const atom_t arg,
          const atom_t val)
{
  atom_t ret;
  TRACE_BIND_SEXP(closure);
  TRACE_BIND_SEXP(arg);
  TRACE_BIND_SEXP(val);
  /*
   */
  switch (arg->type) {
    case T_PAIR: {
      /*
       * Grab the CARs, evaluate the value and bind them.
       */
      atom_t sym = lisp_car(arg);
      atom_t vl0 = lisp_car(val);
      atom_t cl0 = lisp_bind(lisp, closure, sym, vl0);
      /*
       * Grab the CDRs and recursively bind them.
       */
      atom_t oth = lisp_cdr(arg);
      atom_t rem = lisp_cdr(val);
      X(arg, val);
      /*
      */
      ret = lisp_bind(lisp, cl0, oth, rem);
      break;
    }
    case T_SYMBOL: {
      ret = lisp_setq(closure, lisp_cons(arg, val));
      X(arg, val);
      TRACE_BIND_SEXP(ret);
      break;
    }
    default: {
      X(arg, val);
      ret = closure;
      break;
    }
  }
  /*
   */
  TRACE_BIND_SEXP(ret);
  return ret;
}

static atom_t
lisp_bind_args(const lisp_t lisp, const atom_t closure, const atom_t env,
               const atom_t args, const atom_t vals, atom_t * const narg,
               atom_t * const nval)
{
  TRACE_BIND_SEXP(closure);
  TRACE_BIND_SEXP(env);
  TRACE_BIND_SEXP(args);
  TRACE_BIND_SEXP(vals);
  /*
   * Return if we run out of arguments.
   */
  if (IS_NULL(args)) {
    *narg = args;
    *nval = vals;
    TRACE_BIND_SEXP(env);
    return env;
  }
  /*
   * If args is a single symbol, bind the unevaluated values to it.
   */
  if (IS_SYMB(args)) {
    atom_t res = lisp_bind(lisp, env, args, vals);
    *narg = UP(NIL);
    *nval = UP(NIL);
    TRACE_BIND_SEXP(res);
    return res;
  }
  /*
   * Return if we run out of values.
   */
  if (IS_NULL(vals)) {
    *narg = args;
    *nval = vals;
    TRACE_BIND_SEXP(env);
    return env;
  }
  /*
   * Grab the CARs, evaluate the value and bind them.
   */
  atom_t sym = lisp_car(args);
  atom_t val = lisp_eval(lisp, closure, lisp_car(vals));
  atom_t cl0 = lisp_bind(lisp, env, sym, val);
  /*
   * Grab the CDRs and recursively bind them.
   */
  atom_t oth = lisp_cdr(args);
  atom_t rem = lisp_cdr(vals);
  X(args, vals);
  /*
  */
  return lisp_bind_args(lisp, closure, cl0, oth, rem, narg, nval);
}

/*
 * Lambda evaluation.
 */

/*
 * NOTE(xrg) Function evaluation rely on a closure stack. Each new context is
 * pushed on the stack before the evaluation of the function.
 *
 * A function's closure contain its currently resolved arguments during
 * currying. This context takes precedence over those in the closure stack.
 */

static atom_t
lisp_eval_func(const lisp_t lisp, const atom_t closure, const atom_t cell,
               atom_t * const rem)
{
  atom_t rslt;
  TRACE_SEXP(cell);
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
  atom_t clos = lisp_car(cdr0);
  atom_t cdr1 = lisp_cdr(cdr0);
  atom_t clst = lisp_car(cdr1);
  atom_t body = lisp_cdr(cdr1);
  X(lbda, cdr0, cdr1);
  /*
   * Bind the arguments and the values. The closure embedded in the lambda
   * is used as the run environment and augmented with the arguments'
   * values. The call-site closure is used for the evaluation of the
   * arguments.
   */
  atom_t narg;
  atom_t next = lisp_bind_args(lisp, closure, clst, args, vals, &narg, rem);
  /*
   * If the list of remaining arguments is not NIL, handle partial
   * application.
   */
  if (!IS_NULL(narg)) {
    atom_t con0 = lisp_cons(next, body);
    atom_t con1 = lisp_cons(clos, con0);
    X(next, body, clos, con0);
    rslt = lisp_cons(narg, con1);
    X(narg, con1);
  }
  /*
   * Evaluation the native function. Native functions have no definition-site
   * closures, so we pass the previously computed closure with the currently
   * available closure to the function.
   */
  else if (IS_NUMB(body)) {
    function_t fun = (function_t)body->number;
    atom_t ncls = IS_NULL(clos) ? UP(closure) : lisp_cons(clos, closure);
    X(clos, body, narg);
    rslt = fun(lisp, ncls, next);
    X(ncls, next);
  }
  /*
   * Stack the closures and the curried arguments in order and evaluate the
   * function as a PROG.
   */
  else {
    atom_t cdup = lisp_dup(clos);
    atom_t con0 = IS_NULL(cdup) ? UP(closure) : lisp_conc(cdup, closure);
    atom_t con1 = IS_NULL(next) ? UP(con0) : lisp_cons(next, con0);
    X(narg, clos, cdup, con0, next);
    rslt = lisp_prog(lisp, con1, body, UP(NIL));
    X(con1);
  }
  /*
   */
  TRACE_SEXP(rslt);
  return rslt;
}

/*
 * List evaluation.
 */

static atom_t
lisp_eval_pair(const lisp_t lisp, const atom_t closure, const atom_t cell)
{
  atom_t rslt, rem;
  TRACE_SEXP(cell);
  /*
   * Handle the case when CAR is a function.
   */
  if (IS_FUNC(CAR(cell))) {
    rslt = lisp_eval_func(lisp, closure, cell, &rem);
  }
  /*
   * If it's a symbol, re-evaluate cell.
   */
  else if (IS_SYMB(CAR(cell))) {
    rslt = lisp_eval(lisp, closure, cell);
    rem = UP(NIL);
  }
  /*
   * Otherwise, just return the cell.
   */
  else {
    rslt = cell;
    rem = UP(NIL);
  }
  /*
   * Check if there is any remainder arguments.
   */
  if (IS_NULL(rem)) {
    X(rem);
    TRACE_SEXP(rslt);
    return rslt;
  }
  /*
   * Apply the result to the remainder arguments.
   */
  atom_t old = rslt;
  rslt = lisp_cons(old, rem);
  X(old, rem);
  /*
   */
  return lisp_eval_pair(lisp, closure, rslt);
}

/*
 * Generic evaluation.
 */

atom_t
lisp_eval(const lisp_t lisp, const atom_t closure, const atom_t cell)
{
  atom_t rslt;
  TRACE_SEXP(cell);
  /*
   */
  switch (cell->type) {
    case T_PAIR: {
      /*
       * Evaluate CAR.
       */
      atom_t car = lisp_eval(lisp, closure, lisp_car(cell));
      atom_t cdr = lisp_cdr(cell);
      X(cell);
      /*
       * Build the new value.
       */
      atom_t new = lisp_cons(car, cdr);
      X(car, cdr);
      /*
       * Evaluate the list.
       */
      rslt = lisp_eval_pair(lisp, closure, new);
      break;
    }
    case T_SYMBOL: {
      rslt = lisp_lookup(lisp, closure, cell);
      X(cell);
      break;
    }
    default: {
      rslt = cell;
      break;
    }
  }
  /*
   */
  TRACE_SEXP(rslt);
  return rslt;
}

/*
 * Print function.
 */

#define IO_BUFFER_LEN 1024

static size_t
lisp_prin_atom(FILE* const handle, char * const buf, const size_t idx,
               const atom_t closure, const atom_t cell, const bool s);

static size_t
lisp_write(FILE* const handle, char * const buf, const size_t idx,
           void * data, const size_t len)
{
  size_t pidx = idx;
  /*
   * Flush the buffer if necessary.
   */
  if (pidx + len >= IO_BUFFER_LEN) {
    fwrite(buf, 1, pidx, handle);
    pidx = 0;
  }
  /*
   * Append the new data.
   */
  memcpy(&buf[pidx], data, len);
  return pidx + len;
}

static void
lisp_flush(FILE* const handle, char * const buf, const size_t idx)
{
  fwrite(buf, 1, idx, handle);
  fflush(handle);
}

static size_t
lisp_prin_pair(FILE* const handle, char * const buf, const size_t idx,
               const atom_t closure, const atom_t cell, const bool s)
{
  size_t nxt = 0;
  /*
   * Print CAR.
   */
  nxt = lisp_prin_atom(handle, buf, idx, closure, CAR(cell), s);
  /*
   * Print CDR.
   */
  if (!IS_NULL(CDR(cell))) {
    if (IS_PAIR(CDR(cell))) {
      if (s) nxt = lisp_write(handle, buf, nxt, " ", 1);
      return lisp_prin_pair(handle, buf, nxt, closure, CDR(cell), s);
    }
    else {
      if (s) nxt = lisp_write(handle, buf, nxt, " . ", 3);
      return lisp_prin_atom(handle, buf, nxt, closure, CDR(cell), s);
    }
  }
  /*
   */
  return nxt;
}

static size_t
lisp_prin_atom(FILE* const handle, char * const buf, const size_t idx,
               const atom_t closure, const atom_t cell, const bool s)
{
  switch (cell->type) {
    case T_NIL:
      return s ? lisp_write(handle, buf, idx, "NIL", 3) : 0;
    case T_TRUE:
      return lisp_write(handle, buf, idx, "T", 1);
    case T_CHAR: {
      char c = (char)cell->number;
      if (s) {
        size_t nxt = lisp_write(handle, buf, idx, "^", 1);
        switch (c) {
          case '\033':
            nxt = lisp_write(handle, buf, nxt, "\\e", 2);
            break;
          case '\n':
            nxt = lisp_write(handle, buf, nxt, "\\n", 2);
            break;
          case '\r':
            nxt = lisp_write(handle, buf, nxt, "\\r", 2);
            break;
          case '\t':
            nxt = lisp_write(handle, buf, nxt, "\\t", 2);
            break;
          default:
            nxt = lisp_write(handle, buf, nxt, &c, 1);
            break;
        }
        return nxt;
      }
      return lisp_write(handle, buf, idx, &c, 1);
    }
    case T_PAIR: {
      size_t nxt = idx;
      if (s) nxt = lisp_write(handle, buf, nxt, "(", 1);
      nxt = lisp_prin_pair(handle, buf, nxt, closure, cell, s);
      if (s) nxt = lisp_write(handle, buf, nxt, ")", 1);
      return nxt;
    }
    case T_NUMBER: {
      char buffer[24] = { 0 };
#if defined(__MACH__) || defined(__OpenBSD__)
      sprintf(buffer, "%lld", cell->number);
#else
      sprintf(buffer, "%ld", cell->number);
#endif
      return lisp_write(handle, buf, idx, buffer, strlen(buffer));
    }
    case T_SYMBOL:
      return lisp_write(handle, buf, idx, cell->symbol.val,
                        strnlen(cell->symbol.val, LISP_SYMBOL_LENGTH));
    case T_WILDCARD:
      return lisp_write(handle, buf, idx, "_", 1);
    default:
      return 0;
  }
}

void
lisp_prin(const atom_t closure, const atom_t cell, const bool s)
{
  FILE* handle = (FILE*)CAR(CAR(OCHAN))->number;
  char buffer[IO_BUFFER_LEN];
  size_t idx = lisp_prin_atom(handle, buffer, 0, closure, cell, s);
  lisp_flush(handle, buffer, idx);
}

// vim: tw=80:sw=2:ts=2:sts=2:et
