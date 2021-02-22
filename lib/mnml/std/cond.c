#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_cond(const lisp_t lisp, const atom_t closure, const atom_t cell,
          const atom_t match)
{
  /*
   * Sanity checks.
   */
  if (IS_NULL(match) || !IS_PAIR(match) || !IS_PAIR(CAR(match))) {
    X(lisp->slab, cell, match);
    return lisp_make_nil(lisp);
  }
  /*
   * Get CAR/CDR.
   */
  atom_t car = lisp_car(lisp, match);
  atom_t cdr = lisp_cdr(lisp, match);
  X(lisp->slab, match);
  /*
   * Get args and prog.
   */
  atom_t args = lisp_car(lisp, car);
  atom_t prog = lisp_cdr(lisp, car);
  X(lisp->slab, car);
  /*
   * If the cond argument _, simply execute the program.
   */
  if (IS_WILD(args)) {
    X(lisp->slab, args, cdr, cell);
    return lisp_eval(lisp, closure, prog);
  }
  /*
   * Build the predicate.
   */
  atom_t con = lisp_cons(lisp, UP(cell), lisp_make_nil(lisp));
  atom_t evl = lisp_cons(lisp, args, con);
  /*
   * Evaluate the predicate.
   */
  atom_t res = lisp_eval(lisp, closure, evl);
  /*
   */
  if (IS_TRUE(res)) {
    X(lisp->slab, cdr, cell, res);
    return lisp_eval(lisp, closure, prog);
  }
  /*
   */
  X(lisp->slab, prog, res);
  return lisp_cond(lisp, closure, cell, cdr);
}

static atom_t USED
lisp_function_cond(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, ANY);
  atom_t car = lisp_car(lisp, ANY);
  atom_t cdr = lisp_cdr(lisp, ANY);
  atom_t res = lisp_cond(lisp, C, car, cdr);
  return res;
}

LISP_MODULE_SETUP(cond, cond, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
