#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
      atom_t kvp = lisp_cons(arg, val);
      ret = lisp_cons(kvp, closure);
      X(arg, val, kvp, closure);
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
lisp_bind_args(const lisp_t lisp, const atom_t cscl, const atom_t dscl,
               const atom_t args, const atom_t vals)
{
  TRACE_BIND_SEXP(args);
  TRACE_BIND_SEXP(vals);
  /*
   * Return (DSCL, NIL, VALS) if we run out of arguments. It can also be
   * (DSCL, NIL, NIL) if there is no more values either.
   */
  if (IS_NULL(args)) {
    atom_t tail = lisp_cons(NIL, vals);
    atom_t rslt = lisp_cons(dscl, tail);
    X(dscl, tail, args, vals);
    TRACE_BIND_SEXP(rslt);
    return rslt;
  }
  /*
   * If ARGS is a single symbol, bind the unevaluated values to it. That
   * operation consumes all the values, so it returns (DSCL, NIL NIL).
   */
  if (IS_SYMB(args)) {
    atom_t head = lisp_bind(lisp, dscl, args, vals);
    atom_t tail = lisp_cons(NIL, NIL);
    atom_t rslt = lisp_cons(head, tail);
    X(head, tail);
    TRACE_BIND_SEXP(rslt);
    return rslt;
  }
  /*
   * Return (DSCL, ARGS, NIL) if we run out of values.
   */
  if (IS_NULL(vals)) {
    atom_t tail = lisp_cons(args, NIL);
    atom_t rslt = lisp_cons(dscl, tail);
    X(dscl, tail, args, vals);
    TRACE_BIND_SEXP(rslt);
    return rslt;
  }
  /*
   * If there is an ARG and a VAL available, we grab the CAR of each and we
   * evaluate the value within the call-site closure.
   */
  atom_t arg = lisp_car(args);
  atom_t val = lisp_eval(lisp, cscl, lisp_car(vals));
  atom_t oth = lisp_cdr(args);
  atom_t rem = lisp_cdr(vals);
  X(args, vals);
  /*
   * Then we bind the value to the argument. We do that to handle pattern
   * decomposition for non-symbolic arguments.
   */
  atom_t bind = lisp_bind(lisp, dscl, arg, val);
  /*
   * Perform a recursive descent on the remaining arguments and values.
   */
  atom_t rslt = lisp_bind_args(lisp, cscl, bind, oth, rem);
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
   * Grab the arguments, the definition-site closure and the body of the lambda.
   */
  atom_t args = lisp_car(func);
  atom_t cdr0 = lisp_cdr(func);
  atom_t dscl = lisp_car(cdr0);
  atom_t body = lisp_cdr(cdr0);
  X(func, cdr0);
  /*
   * Bind the arguments and the values. The closure embedded in the lambda is
   * used as the run environment and augmented with the values of the arguments.
   * The call-site closure is used for the evaluation of the arguments.
   */
  atom_t bind = lisp_bind_args(lisp, closure, dscl, args, vals);
  atom_t bscl = lisp_car(bind);
  atom_t cdr1 = lisp_cdr(bind);
  atom_t narg = lisp_car(cdr1);
  atom_t nval = lisp_cdr(cdr1);
  X(bind, cdr1);
  /*
   * If no argument remains, apply the function.
   */
  if (IS_NULL(narg)) {
    /*
     * Prepend the bind-site closure to the call-site closure.
     */
    atom_t dup = lisp_dup(bscl);
    atom_t cls = lisp_conc(dup, closure);
    X(bscl, dup);
    /*
     * Evaluation the native function. Native functions have no definition-site
     * closures, so we pass the previously computed closure with the currently
     * available closure to the function.
     */
    if (IS_NUMB(body)) {
      function_t fun = (function_t)body->number;
      rslt = fun(lisp, cls);
      X(body);
    }
    /*
     * Stack the closures and the curried arguments in order and evaluate the
     * function as a PROG.
     */
    else {
      rslt = lisp_prog(lisp, cls, body, UP(NIL));
    }
    /*
     * Clean-up.
     */
    X(narg, cls);
  }
  /*
   * Else handle partial application.
   */
  else {
    atom_t con0 = lisp_cons(bscl, body);
    rslt = lisp_cons(narg, con0);
    X(narg, con0, bscl, body);
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
  TRACE_CLOS_SEXP(closure);
  TRACE_EVAL_SEXP(rslt);
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

// vim: tw=80:sw=2:ts=2:sts=2:et