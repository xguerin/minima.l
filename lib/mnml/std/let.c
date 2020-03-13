#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t
lisp_let_bind(const lisp_t lisp, const atom_t closure, const atom_t env,
              const atom_t cell)
{
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
    return env;
  }
  /*
   * Temporarily append this current environment to the evaluation closure.
   */
  atom_t tmp = lisp_merge(UP(env), lisp_dup(closure));
  atom_t arg = lisp_car(car);
  atom_t val = lisp_eval(lisp, tmp, lisp_cdr(car));
  X(tmp, car);
  /*
   * Bind the result to the environment and process the remainder.
   */
  atom_t next = lisp_bind(lisp, env, arg, val);
  return lisp_let_bind(lisp, closure, next, cdr);
}

static atom_t
lisp_let(const lisp_t lisp, const atom_t closure, const atom_t cell)
{
  /*
   * Get the bind list and the prog.
   */
  atom_t bind = lisp_car(cell);
  atom_t prog = lisp_cdr(cell);
  X(cell);
  /*
   * Recursively apply the bind list.
   */
  atom_t next = lisp_let_bind(lisp, closure, UP(NIL), bind);
  atom_t clos = lisp_merge(next, lisp_dup(closure));
  /*
   * Evaluate the prog with the new bind list.
   */
  atom_t res = lisp_prog(lisp, clos, prog, UP(NIL));
  X(clos);
  return res;
}

static atom_t USED
lisp_function_let(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, cell, closure, @);
  return lisp_let(lisp, closure, cell);
}

LISP_MODULE_SETUP(let, let, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
