#include "mnml/debug.h"
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
   * Prepend this current environment to the evaluation closure.
   */
  atom_t dup = lisp_dup(env);
  atom_t tmp = lisp_conc(dup, UP(closure));
  atom_t arg = lisp_car(car);
  atom_t nvl = lisp_cdr(car);
  X(car);
  /*
   * Evaluate the value. and bind it to the environment.
   */
  atom_t val = lisp_eval(lisp, tmp, nvl);
  X(tmp);
  /*
   * Bind it to the environment.
   */
  atom_t nxt = lisp_bind(lisp, env, arg, val);
  /*
   * Process the remainder.
   */
  return lisp_let_bind(lisp, closure, nxt, cdr);
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
  atom_t clos = lisp_conc(next, UP(closure));
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
  LISP_ARGS(closure, C, ANY);
  return lisp_let(lisp, C, UP(ANY));
}

LISP_MODULE_SETUP(let, let, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
