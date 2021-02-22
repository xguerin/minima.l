#include <mnml/debug.h>
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
    X(lisp->slab, cell);
    return env;
  }
  /*
   * Grab CAR and CDR.
   */
  atom_t car = lisp_car(lisp, cell);
  atom_t cdr = lisp_cdr(lisp, cell);
  X(lisp->slab, cell);
  /*
   * Return if the element is not a pair.
   */
  if (unlikely(!IS_PAIR(car))) {
    X(lisp->slab, car, cdr);
    return env;
  }
  /*
   * Prepend this current environment to the evaluation closure.
   */
  atom_t dup = lisp_dup(lisp, env);
  atom_t tmp = lisp_conc(lisp, dup, UP(closure));
  atom_t arg = lisp_car(lisp, car);
  atom_t nvl = lisp_cdr(lisp, car);
  X(lisp->slab, car);
  /*
   * Evaluate the value and bind it to the environment.
   */
  atom_t val = lisp_eval(lisp, tmp, nvl);
  atom_t nxt = lisp_bind(lisp, env, arg, val);
  /*
   * If the value is a function, mark the tail calls.
   */
  if (IS_SYMB(arg) && IS_FUNC(val)) {
    bool unique = true;
    /*
     * Check that the symbol is unique in the definition-site closure. If it is
     * not unique, the symbol will be resolved to the one in the definition-site
     * closure, and not the one in the call-site closure, as it should for a
     * let-bound recursive lambda.
     */
    FOREACH(tmp, p)
    {
      if (lisp_symbol_match(CAR(p->car), &arg->symbol)) {
        unique = false;
        break;
      }
      NEXT(p);
    }
    /*
     * If it is unique, mark the tail calls.
     */
    if (unique) {
      lisp_mark_tail_calls(lisp, arg, CAR(val), CDR(CDR(val)));
    }
  }
  /*
   * Process the remainder.
   */
  X(lisp->slab, tmp);
  return lisp_let_bind(lisp, closure, nxt, cdr);
}

static atom_t
lisp_let(const lisp_t lisp, const atom_t closure, const atom_t cell)
{
  /*
   * Get the bind list and the prog.
   */
  atom_t bind = lisp_car(lisp, cell);
  atom_t prog = lisp_cdr(lisp, cell);
  X(lisp->slab, cell);
  /*
   * Recursively apply the bind list.
   */
  atom_t next = lisp_let_bind(lisp, closure, lisp_make_nil(lisp), bind);
  atom_t clos = lisp_conc(lisp, next, UP(closure));
  /*
   * Evaluate the prog with the new bind list.
   */
  atom_t res = lisp_prog(lisp, clos, prog, lisp_make_nil(lisp));
  X(lisp->slab, clos);
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
