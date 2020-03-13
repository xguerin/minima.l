#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static bool
lisp_eval_cond(const lisp_t lisp, const atom_t closure, const atom_t cell)
{
  UP(cell);
  atom_t evl = lisp_eval(lisp, closure, cell);
  const bool res = !IS_NULL(evl);
  X(evl);
  return res;
}

static atom_t USED
lisp_function_while(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, args, closure, @);
  /*
   * Extract the condition and the prog.
   */
  atom_t cnd = lisp_car(args);
  atom_t prg = lisp_cdr(args);
  X(args);
  /*
   * Execute the loop.
   */
  atom_t res = UP(NIL);
  while (lisp_eval_cond(lisp, closure, cnd)) {
    X(res);
    UP(prg);
    res = lisp_prog(lisp, closure, prg, UP(NIL));
  }
  /*
   * Clean-up and return the result.
   */
  X(cnd, prg);
  return res;
}

LISP_MODULE_SETUP(while, while, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
