#include <mnml/types.h>
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

static atom_t
lisp_eval_args(const lisp_t lisp, const atom_t closure, const atom_t atom,
               const atom_t args, const atom_t vals)
{
  atom_t rslt;
  TRACE_BIND_SEXP(args);
  TRACE_BIND_SEXP(vals);
  /*
   * Return (DSCL, VALS) if we run out of arguments. It can also be
   * (DSCL, NIL) if there is no more values either.
   */
  if (IS_NULL(args)) {
    rslt = lisp_cons(lisp, atom, args);
    X(lisp, vals);
  }
  /*
   * If ARGS is a single symbol, bind the unevaluated values to it. That
   * operation consumes all the values, so it returns (DSCL, NIL NIL).
   */
  else if (IS_SYMB(args)) {
    atom_t head = lisp_bind(lisp, atom, args, vals);
    rslt = lisp_cons(lisp, head, lisp_make_nil(lisp));
  }
  /*
   * Return (DSCL, ARGS, NIL) if we run out of values.
   */
  else if (IS_NULL(vals)) {
    rslt = lisp_cons(lisp, atom, args);
    X(lisp, vals);
  }
  /*
   * If there is an ARG and a VAL available, we grab the CAR of each and we
   * evaluate the value within the call-site closure.
   */
  else {
    /*
     * Grab CAR and CDR from the arguments.
     */
    atom_t arg = lisp_car(lisp, args);
    atom_t oth = lisp_cdr(lisp, args);
    X(lisp, args);
    /*
     * Grab CAR and CDR from the values, evaluate CAR.
     */
    atom_t val = lisp_eval(lisp, closure, lisp_car(lisp, vals));
    atom_t rem = lisp_cdr(lisp, vals);
    X(lisp, vals);
    /*
     * Then we bind the value to the argument. We do that to handle pattern
     * decomposition for non-symbolic arguments. Perform a recursive descent on
     * the remaining arguments and values.
     */
    atom_t bind = lisp_bind(lisp, atom, arg, val);
    rslt = lisp_eval_args(lisp, closure, bind, oth, rem);
  }
  /*
   * Return the result.
   */
  TRACE_BIND_SEXP(rslt);
  return rslt;
}

/*
 * Function evaluation. A function's closure contains its currently resolved
 * arguments during currying.
 */

static atom_t
lisp_eval_func(const lisp_t lisp, const atom_t closure, const atom_t symb,
               const atom_t func, const atom_t vals)
{
  atom_t rslt;
  TRACE_CLOS_SEXP(closure);
  TRACE_EVAL_SEXP(func);
  /*
   * Grab the arguments.
   */
  atom_t args = lisp_car(lisp, func);
  /*
   * Check if the function can be applied.
   */
  if (!lisp_may_apply(args, vals)) {
    X(lisp, func, vals, args);
    return lisp_make_nil(lisp);
  }
  /*
   * Grab the definition-site closure and the body.
   */
  atom_t cdr0 = lisp_cdr(lisp, func);
  atom_t dscl = lisp_car(lisp, cdr0);
  atom_t body = lisp_cdr(lisp, cdr0);
  X(lisp, func, cdr0);
  /*
   * Bind the arguments and the values.
   */
  atom_t bind = lisp_eval_args(lisp, closure, lisp_make_nil(lisp), args, vals);
  /*
   * Extract the bound arguments, and remaining arguments and values,
   */
  atom_t bscl = lisp_car(lisp, bind);
  atom_t narg = lisp_cdr(lisp, bind);
  X(lisp, bind);
  /*
   * Evaluate the function if all the arguments were bound.
   */
  if (IS_NULL(narg)) {
    /*
     * Evaluate the binary function.
     */
    if (IS_NUMB(body)) {
      /*
       * In the case of binary functions, the embedded closure only contain
       * curried arguments. Therefore, we just append those to the closure.
       */
      atom_t clos = bscl;
      if (!IS_NULL(dscl)) {
        clos = lisp_conc(lisp, bscl, lisp_dup(lisp, dscl));
      }
      clos = lisp_conc(lisp, clos, UP(closure));
      X(lisp, dscl);
      /*
       * Call the binary function.
       */
      function_t fun = (function_t)body->number;
      rslt = fun(lisp, clos);
      X(lisp, body, narg, clos);
    }
    /*
     * Return the (SYMB, BSCL) tail-call for further evaluation.
     */
    else if (IS_TAIL_CALL(symb)) {
      rslt = lisp_cons(lisp, UP(symb), bscl);
      SET_TAIL_CALL(rslt);
      X(lisp, body, dscl, narg);
    }
    /*
     * Evaluate the lisp function.
     */
    else {
      /*
       * Merge the definition-site closure first.
       */
      atom_t cls0 = lisp_merge(lisp, lisp_dup(lisp, closure), dscl);
      X(lisp, dscl);
      /*
       * Tail-call evaluation loop.
       */
      while (true) {
        /*
         * Merge the bind-site closure.
         */
        atom_t cls1 = lisp_merge(lisp, lisp_dup(lisp, cls0), bscl);
        /*
         * Evaluate the function's body.
         */
        atom_t res = lisp_prog(lisp, cls1, UP(body), lisp_make_nil(lisp));
        X(lisp, cls1);
        /*
         * If the result is not a tail call, stop the evaluation.
         */
        if (!IS_TAIL_CALL(res) || !lisp_symbol_match(CAR(res), &symb->symbol)) {
          rslt = res;
          break;
        }
        /*
         * If it's a tail call, evaluate the function with the new arguments.
         */
        X(lisp, bscl);
        bscl = lisp_cdr(lisp, res);
        X(lisp, res);
      }
      /*
       * Done.
       */
      X(lisp, body, bscl, narg, cls0);
    }
  }
  /*
   * Else handle partial application.
   */
  else {
    atom_t clos = lisp_conc(lisp, bscl, dscl);
    atom_t con0 = lisp_cons(lisp, clos, body);
    rslt = lisp_cons(lisp, narg, con0);
  }
  /*
   */
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
   * Grab CAR and CDR.
   */
  atom_t car = lisp_car(lisp, cell);
  atom_t cdr = lisp_cdr(lisp, cell);
  X(lisp, cell);
  /*
   * Evaluate CAR.
   */
  atom_t nxt = lisp_eval(lisp, closure, UP(car));
  /*
   * Handle the case when CAR is a function.
   */
  if (IS_FUNC(nxt)) {
    rslt = lisp_eval_func(lisp, closure, car, nxt, cdr);
  }
  /*
   * If CAR and CNR are the same, recompose the list.
   */
  else if (lisp_equ(car, nxt)) {
    rslt = lisp_cons(lisp, nxt, cdr);
  }
  /*
   * If it's a symbol, re-evaluate cell.
   */
  else if (IS_SYMB(nxt) || IS_PAIR(nxt)) {
    rslt = lisp_eval(lisp, closure, lisp_cons(lisp, nxt, cdr));
  }
  /*
   * Otherwise, recompose the list.
   */
  else {
    rslt = lisp_cons(lisp, nxt, cdr);
  }
  X(lisp, car);
  /*
   */
  TRACE_EVAL_SEXP(rslt);
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
      rslt = lisp_lookup(lisp, closure, cell);
      X(lisp, cell);
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
