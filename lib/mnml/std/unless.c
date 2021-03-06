#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_unless(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, COND, REM);
  /*
   * Evaluate REM branch if TRUE.
   */
  if (likely(IS_NULL(COND))) {
    return lisp_prog(lisp, C, UP(REM), lisp_make_nil(lisp));
  }
  /*
   * Or return NIL;
   */
  return lisp_make_nil(lisp);
}

LISP_MODULE_SETUP(unless, unless, COND, REM)

// vim: tw=80:sw=2:ts=2:sts=2:et
