#include <mnml/debug.h>
#include <mnml/lexer.h>
#include <mnml/lisp.h>
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

atom_t ICHAN = NULL;
atom_t OCHAN = NULL;
atom_t NIL = NULL;
atom_t TRUE = NULL;
atom_t QUOTE = NULL;
atom_t WILDCARD = NULL;

/*
 * Symbol lookup.
 */

atom_t
lisp_lookup(const lisp_t lisp, const atom_t closure, const symbol_t sym)
{
  /*
   * Look for the symbol the closure.
   */
  FOREACH(closure, a)
  {
    atom_t car = a->car;
    if (lisp_symbol_match(CAR(car), sym)) {
      return UP(CDR(car));
    }
    NEXT(a);
  }
  /*
   * Check the global environemnt.
   */
  FOREACH(lisp->GLOBALS, g)
  {
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
  } else {
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
  FILE* handle = (FILE*)hnd->number;
  /*
   */
  char buffer[RBUFLEN] = { 0 };
  do {
    char* p = fgets(buffer + lexer.rem, RBUFLEN - lexer.rem, handle);
    if (p == NULL) {
      break;
    }
    size_t len = strlen(p);
    lisp_lexer_parse(&lexer, buffer, lexer.rem + len,
                     lexer.rem + len < RBUFLEN);
  } while (CDR(CDR(CAR(ICHAN))) == NIL || lisp_lexer_pending(&lexer));
  /*
   * Grab the result and return it.
   */
  lisp_lexer_destroy(&lexer);
  return CDR(CDR(chn)) != NIL ? lisp_read_pop() : NULL;
}

/*
 * SETQ. PAIR is consumed.
 */

atom_t
lisp_setq(const atom_t closure, const atom_t pair)
{
  /*
   * Check if pair is valid.
   */
  if (!IS_PAIR(pair) || !IS_SYMB(CAR(pair))) {
    X(pair);
    return UP(closure);
  }
  /*
   * If the closure is NIL, return the wrapped pair.
   */
  if (IS_NULL(closure)) {
    atom_t res = lisp_cons(pair, closure);
    X(pair);
    return res;
  }
  /*
   * Extract CAR and CDR.
   */
  atom_t car = lisp_car(closure);
  atom_t cdr = lisp_cdr(closure);
  /*
   * Replace car if its symbol matches pair's.
   */
  if (lisp_symbol_match(CAR(car), &CAR(pair)->symbol)) {
    atom_t res = lisp_cons(pair, cdr);
    X(pair, cdr, car);
    return res;
  }
  /*
   * Look further down the closure.
   */
  atom_t nxt = lisp_setq(cdr, pair);
  atom_t res = lisp_cons(car, nxt);
  X(cdr, car, nxt);
  return res;
}

/*
 * PROG. CELL and RESULT are consumed.
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
 * Argument bindings. CLOSURE, ARG and VAL are consumed.
 */

atom_t
lisp_bind(const lisp_t lisp, const atom_t closure, const atom_t arg,
          const atom_t val)
{
  atom_t ret;
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
      TRACE_BIND_SEXP(arg);
      TRACE_BIND_SEXP(val);
      ret = lisp_setq(closure, lisp_cons(arg, val));
      X(closure, arg, val);
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
  return ret;
}

static atom_t
lisp_bind_args(const lisp_t lisp, const atom_t cl0, const atom_t cl1,
               const atom_t args, const atom_t vals)
{
  TRACE_BIND_SEXP(args);
  TRACE_BIND_SEXP(vals);
  /*
   * Return if we run out of arguments.
   */
  if (IS_NULL(args)) {
    atom_t cns = lisp_cons(args, vals);
    atom_t res = lisp_cons(cl1, cns);
    X(cl1, cns, args, vals);
    TRACE_BIND_SEXP(res);
    return res;
  }
  /*
   * If ARGS is a single symbol, bind the unevaluated values to it.
   */
  if (IS_SYMB(args)) {
    atom_t tail = lisp_cons(NIL, NIL);
    atom_t head = lisp_bind(lisp, cl1, args, vals);
    atom_t rslt = lisp_cons(head, tail);
    X(head, tail);
    TRACE_BIND_SEXP(rslt);
    return rslt;
  }
  /*
   * Return if we run out of values.
   */
  if (IS_NULL(vals)) {
    atom_t cns = lisp_cons(args, vals);
    atom_t res = lisp_cons(cl1, cns);
    X(cl1, cns, args, vals);
    TRACE_BIND_SEXP(res);
    return res;
  }
  /*
   * Grab CAR and CDR.
   */
  atom_t sym = lisp_car(args);
  atom_t val = lisp_eval(lisp, cl0, lisp_car(vals));
  atom_t oth = lisp_cdr(args);
  atom_t rem = lisp_cdr(vals);
  X(args, vals);
  /*
   * Recursive descent.
   */
  atom_t next = lisp_bind_args(lisp, cl0, cl1, oth, rem);
  atom_t head = lisp_car(next);
  atom_t tail = lisp_cdr(next);
  X(next);
  /*
   * Bind and return the result.
   */
  atom_t updt = lisp_bind(lisp, head, sym, val);
  atom_t rslt = lisp_cons(updt, tail);
  X(updt, tail);
  TRACE_BIND_SEXP(rslt);
  return rslt;
}

/*
 * Function evaluation. A function's closure contains its currently resolved
 * arguments during currying.
 */

static atom_t
lisp_eval_func(const lisp_t lisp, const atom_t closure, const atom_t func,
               const atom_t vals)
{
  atom_t rslt;
  TRACE_CLOS_SEXP(closure);
  TRACE_EVAL_SEXP(func);
  /*
   * Grab the arguments, the closure and the body of the lambda.
   */
  atom_t args = lisp_car(func);
  atom_t cdr0 = lisp_cdr(func);
  atom_t fcls = lisp_car(cdr0);
  atom_t body = lisp_cdr(cdr0);
  X(func, cdr0);
  /*
   * Bind the arguments and the values. The closure embedded in the lambda is
   * used as the run environment and augmented with the arguments' values.  The
   * call-site closure is used for the evaluation of the arguments.
   */
  atom_t bind = lisp_bind_args(lisp, closure, fcls, args, vals);
  atom_t next = lisp_car(bind);
  atom_t cdr1 = lisp_cdr(bind);
  atom_t narg = lisp_car(cdr1);
  atom_t nval = lisp_cdr(cdr1);
  X(bind, cdr1);
  /*
   * If some arguments remain, handle partial application.
   */
  if (!IS_NULL(narg)) {
    atom_t con0 = lisp_cons(next, body);
    rslt = lisp_cons(narg, con0);
    X(narg, con0, next, body);
  }
  /*
   * Apply the function.
   */
  else {
    /*
     * Merge the definition-site closure with the call-site closure.
     */
    atom_t clos = lisp_merge(next, lisp_dup(closure));
    /*
     * Evaluation the native function. Native functions have no definition-site
     * closures, so we pass the previously computed closure with the currently
     * available closure to the function.
     */
    if (IS_NUMB(body)) {
      function_t fun = (function_t)body->number;
      rslt = fun(lisp, clos);
      X(body);
    }
    /*
     * Stack the closures and the curried arguments in order and evaluate the
     * function as a PROG.
     */
    else {
      rslt = lisp_prog(lisp, clos, body, UP(NIL));
    }
    /*
     * Clean-up.
     */
    X(narg, clos);
  }
  /*
   * If there is any remaining values, append them.
   */
  if (!IS_NULL(nval)) {
    atom_t tmp = rslt;
    rslt = lisp_eval(lisp, closure, lisp_cons(rslt, nval));
    X(tmp);
  }
  /*
   */
  X(nval);
  return rslt;
}

/*
 * List evaluation.
 */

static atom_t
lisp_eval_pair(const lisp_t lisp, const atom_t closure, const atom_t cell)
{
  atom_t rslt;
  TRACE_EVAL_SEXP(cell);
  /*
   * Evaluate CAR.
   */
  atom_t car = lisp_car(cell);
  atom_t nxt = lisp_eval(lisp, closure, UP(car));
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  /*
   * Handle the case when CAR is a function.
   */
  if (IS_FUNC(nxt)) {
    rslt = lisp_eval_func(lisp, closure, nxt, cdr);
  }
  /*
   * If CAR and CNR are the same, recompose the list.
   */
  else if (lisp_equ(car, nxt)) {
    rslt = lisp_cons(nxt, cdr);
    X(nxt, cdr);
  }
  /*
   * If it's a symbol, re-evaluate cell.
   */
  else if (IS_SYMB(nxt) || IS_PAIR(nxt)) {
    rslt = lisp_eval(lisp, closure, lisp_cons(nxt, cdr));
    X(nxt, cdr);
  }
  /*
   * Otherwise, recompose the list.
   */
  else {
    rslt = lisp_cons(nxt, cdr);
    X(nxt, cdr);
  }
  /*
   */
  X(car);
  return rslt;
}

/*
 * Generic evaluation.
 */

atom_t
lisp_eval(const lisp_t lisp, const atom_t closure, const atom_t cell)
{
  atom_t rslt;
  TRACE_EVAL_SEXP(cell);
  /*
   */
  switch (cell->type) {
    case T_PAIR: {
      rslt = lisp_eval_pair(lisp, closure, cell);
      break;
    }
    case T_SYMBOL: {
      rslt = lisp_lookup(lisp, closure, &cell->symbol);
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
  TRACE_EVAL_SEXP(rslt);
  return rslt;
}

/*
 * Print function.
 */

#define IO_BUFFER_LEN 1024

static size_t lisp_prin_atom(FILE* const handle, char* const buf,
                             const size_t idx, const atom_t closure,
                             const atom_t cell, const bool s);

static size_t
lisp_write(FILE* const handle, char* const buf, const size_t idx, void* data,
           const size_t len)
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
lisp_flush(FILE* const handle, char* const buf, const size_t idx)
{
  fwrite(buf, 1, idx, handle);
  fflush(handle);
}

static size_t
lisp_prin_pair(FILE* const handle, char* const buf, const size_t idx,
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
      if (s)
        nxt = lisp_write(handle, buf, nxt, " ", 1);
      return lisp_prin_pair(handle, buf, nxt, closure, CDR(cell), s);
    } else {
      if (s)
        nxt = lisp_write(handle, buf, nxt, " . ", 3);
      return lisp_prin_atom(handle, buf, nxt, closure, CDR(cell), s);
    }
  }
  /*
   */
  return nxt;
}

static size_t
lisp_prin_atom(FILE* const handle, char* const buf, const size_t idx,
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
      if (s)
        nxt = lisp_write(handle, buf, nxt, "(", 1);
      nxt = lisp_prin_pair(handle, buf, nxt, closure, cell, s);
      if (s)
        nxt = lisp_write(handle, buf, nxt, ")", 1);
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
