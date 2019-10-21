#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t
lisp_let_bind(const atom_t closure, const atom_t env, const atom_t cell)
{
  TRACE_SEXP(cell);
  /*
   * Check if the cell is a pair.
   */
  if (unlikely(!IS_PAIR(cell))) {
    X(cell);
    return env;
  }
  /*
   * Grab CAR and CDR.
   */
  atom_t car = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  /*
   * Return if the element is not a pair.
   */
  if (unlikely(!IS_PAIR(car))) {
    X(car, cdr);
    TRACE_SEXP(env);
    return env;
  }
  /*
   * Temporarily append this current environment to the evaluation closure.
   */
  atom_t tmp = lisp_cons(env, closure);
  atom_t arg = lisp_car(car);
  atom_t val = lisp_eval(tmp, lisp_cdr(car));
  X(tmp, car);
  /*
   * Bind the result to the environment and process the remainder.
   */
  atom_t next = lisp_bind(env, arg, val);
  return lisp_let_bind(closure, next, cdr);
}

static atom_t
lisp_let(const atom_t closure, const atom_t cell)
{
  TRACE_SEXP(cell);
  /*
   * Get the bind list and the prog.
   */
  atom_t bind = lisp_car(cell);
  atom_t prog = lisp_cdr(cell);
  X(cell);
  /*
   * Recursively apply the bind list and queue it up in the closure list.
   */
  atom_t next = lisp_let_bind(closure, UP(NIL), bind);
  atom_t clo = lisp_cons(next, closure);
  X(next);
  /*
   * Evaluate the prog with the new bind list.
   */
  atom_t res = lisp_prog(clo, prog, UP(NIL));
  X(clo);
  return res;
}

static atom_t
lisp_function_let(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(cell, arguments, @);
  return lisp_let(closure, cell);
}

LISP_PLUGIN_REGISTER(let, let, @)
