#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_function_when(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, cnd, closure, COND);
  LISP_LOOKUP(lisp, prg, closure, REM);
  /*
   * Evaluate REM branch if TRUE.
   */
  if (likely(!IS_NULL(cnd))) {
    atom_t res = lisp_prog(lisp, closure, prg, UP(NIL));
    X(cnd);
    return res;
  }
  /*
   * Or return NIL;
   */
  X(cnd, prg);
  return UP(NIL);
}

LISP_MODULE_SETUP(when, when, COND, REM)

// vim: tw=80:sw=2:ts=2:sts=2:et
