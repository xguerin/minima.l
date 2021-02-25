#include <mnml/maker.h>
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

atom_t
lisp_bind(const lisp_t lisp, const atom_t closure, const atom_t arg,
          const atom_t val)
{
  atom_t ret;
  TRACE_BIND_SEXP(arg);
  TRACE_BIND_SEXP(val);
  /*
   */
  switch (arg->type) {
    case T_PAIR: {
      /*
       * Grab CARs and CDR, and clean-up.
       */
      atom_t sym = lisp_car(lisp, arg);
      atom_t vl0 = lisp_car(lisp, val);
      atom_t oth = lisp_cdr(lisp, arg);
      atom_t rem = lisp_cdr(lisp, val);
      X(lisp->slab, arg, val);
      /*
       * Bind CARs and process CDRs.
       */
      atom_t cl0 = lisp_bind(lisp, closure, sym, vl0);
      ret = lisp_bind(lisp, cl0, oth, rem);
      break;
    }
    case T_SYMBOL: {
      atom_t kvp = lisp_cons(lisp, arg, val);
      ret = lisp_cons(lisp, kvp, closure);
      break;
    }
    default: {
      X(lisp->slab, arg, val);
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
lisp_bind_args(const lisp_t lisp, const atom_t cscl, const atom_t dscl,
               const atom_t args, const atom_t vals)
{
  atom_t rslt;
  TRACE_BIND_SEXP(args);
  TRACE_BIND_SEXP(vals);
  /*
   * Return (DSCL, NIL, VALS) if we run out of arguments. It can also be
   * (DSCL, NIL, NIL) if there is no more values either.
   */
  if (IS_NULL(args)) {
    rslt = lisp_cons(lisp, dscl, args);
    X(lisp->slab, vals);
  }
  /*
   * If ARGS is a single symbol, bind the unevaluated values to it. That
   * operation consumes all the values, so it returns (DSCL, NIL NIL).
   */
  else if (IS_SYMB(args)) {
    atom_t head = lisp_bind(lisp, dscl, args, vals);
    rslt = lisp_cons(lisp, head, lisp_make_nil(lisp));
  }
  /*
   * Return (DSCL, ARGS, NIL) if we run out of values.
   */
  else if (IS_NULL(vals)) {
    rslt = lisp_cons(lisp, dscl, args);
    X(lisp->slab, vals);
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
    X(lisp->slab, args);
    /*
     * Grab CAR and CDR from the values, evaluate CAR.
     */
    atom_t val = lisp_eval(lisp, cscl, lisp_car(lisp, vals));
    atom_t rem = lisp_cdr(lisp, vals);
    X(lisp->slab, vals);
    /*
     * Then we bind the value to the argument. We do that to handle pattern
     * decomposition for non-symbolic arguments. Perform a recursive descent on
     * the remaining arguments and values.
     */
    atom_t bind = lisp_bind(lisp, dscl, arg, val);
    rslt = lisp_bind_args(lisp, cscl, bind, oth, rem);
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
    X(lisp->slab, func, vals, args);
    return lisp_make_nil(lisp);
  }
  /*
   * Grab the definition-site closure and the body.
   */
  atom_t cdr0 = lisp_cdr(lisp, func);
  atom_t dscl = lisp_car(lisp, cdr0);
  atom_t body = lisp_cdr(lisp, cdr0);
  X(lisp->slab, func, cdr0);
  /*
   * Bind the arguments and the values. The closure embedded in the lambda is
   * used as the run environment and augmented with the values of the arguments.
   * The call-site closure is used for the evaluation of the arguments.
   */
  atom_t bind = lisp_bind_args(lisp, closure, dscl, args, vals);
  /*
   * Extract the bound arguments, and remaining arguments and values,
   */
  atom_t bscl = lisp_car(lisp, bind);
  atom_t narg = lisp_cdr(lisp, bind);
  X(lisp->slab, bind);
  /*
   * Evaluate the binary function.
   */
  if (IS_NULL(narg) && IS_NUMB(body)) {
    /*
     * Prepend the bind-site closure to the call-site closure.
     */
    atom_t dup = lisp_dup(lisp, bscl);
    atom_t cls = lisp_conc(lisp, dup, UP(closure));
    X(lisp->slab, bscl);
    /*
     * Call the binary function.
     */
    function_t fun = (function_t)body->number;
    rslt = fun(lisp, cls);
    X(lisp->slab, body, narg, cls);
  }
  /*
   * Evaluate the lisp function (tail call).
   */
  else if (IS_NULL(narg) && IS_TAIL_CALL(symb)) {
    rslt = lisp_cons(lisp, UP(symb), lisp_dup(lisp, bscl));
    rslt->flags = F_TAIL_CALL;
    X(lisp->slab, body, narg, bscl);
  }
  /*
   * Evaluate the lisp function.
   */
  else if (IS_NULL(narg)) {
    /*
     * Prepend the bind-site closure to the call-site closure.
     */
    atom_t dup = lisp_dup(lisp, bscl);
    X(lisp->slab, narg, bscl);
    /*
     * Tail-call evaluation loop.
     */
  tailcall_loop:;
    atom_t cls = lisp_conc(lisp, dup, UP(closure));
    atom_t res = lisp_prog(lisp, cls, UP(body), lisp_make_nil(lisp));
    X(lisp->slab, cls);
    /*
     * If it's a tail call, evaluate the function with the new arguments.
     */
    if (IS_TAIL_CALL(res) && lisp_symbol_match(CAR(res), &symb->symbol)) {
      dup = lisp_cdr(lisp, res);
      X(lisp->slab, res);
      goto tailcall_loop;
    }
    /*
     * Otherwise, leave the loop.
     */
    rslt = res;
    X(lisp->slab, body);
  }
  /*
   * Else handle partial application.
   */
  else {
    atom_t con0 = lisp_cons(lisp, bscl, body);
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
  X(lisp->slab, cell);
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
  else if (IS_SCOP(nxt) || IS_SYMB(nxt) || IS_PAIR(nxt)) {
    rslt = lisp_eval(lisp, closure, lisp_cons(lisp, nxt, cdr));
  }
  /*
   * Otherwise, recompose the list.
   */
  else {
    rslt = lisp_cons(lisp, nxt, cdr);
  }
  X(lisp->slab, car);
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
    case T_SCOPED_SYMBOL: {
      atom_t nam = lisp_scope_get_name(lisp, cell);
      atom_t sym = lisp_scope_get_symb(lisp, cell);
      atom_t nil = lisp_make_nil(lisp);
      atom_t scp = lisp_lookup(lisp, lisp->scopes, nil, &nam->symbol);
      rslt = lisp_lookup(lisp, scp, closure, &sym->symbol);
      X(lisp->slab, cell, nam, sym, nil, scp);
      break;
    }
    case T_SYMBOL: {
      rslt = lisp_lookup(lisp, lisp->globals, closure, &cell->symbol);
      X(lisp->slab, cell);
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
