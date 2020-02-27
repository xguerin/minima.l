#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_function_unless(const lisp_t lisp, const atom_t closure,
                     const atom_t arguments)
{
  LISP_LOOKUP(cnd, arguments, COND);
  LISP_LOOKUP(prg, arguments, REM);
  /*
   * Evaluate REM branch if TRUE.
   */
  if (likely(IS_NULL(cnd))) {
    X(cnd);
    return lisp_prog(lisp, closure, prg, UP(NIL));
  }
  /*
   * Or return NIL;
   */
  X(cnd, prg);
  return UP(NIL);
}

LISP_MODULE_SETUP(unless, unless, COND, REM)

// vim: tw=80:sw=2:ts=2:sts=2:et