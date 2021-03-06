#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static bool
lisp_eval_cond(const lisp_t lisp, const atom_t closure, const atom_t cell)
{
  UP(cell);
  atom_t evl = lisp_eval(lisp, closure, cell);
  const bool res = !IS_NULL(evl);
  X(lisp, evl);
  return res;
}

static atom_t USED
lisp_function_while(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, ANY);
  /*
   * Extract the condition and the prog.
   */
  atom_t cnd = lisp_car(lisp, ANY);
  atom_t prg = lisp_cdr(lisp, ANY);
  /*
   * Execute the loop.
   */
  atom_t res = lisp_make_nil(lisp);
  while (lisp_eval_cond(lisp, C, cnd)) {
    X(lisp, res);
    res = lisp_prog(lisp, C, UP(prg), lisp_make_nil(lisp));
  }
  /*
   * Clean-up and return the result.
   */
  X(lisp, cnd, prg);
  return res;
}

LISP_MODULE_SETUP(while, while, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
