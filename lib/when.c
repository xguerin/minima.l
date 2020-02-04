#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_function_when(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(cnd, arguments, COND);
  LISP_LOOKUP(prg, arguments, REM);
  /*
   * Evaluate REM branch if TRUE.
   */
  if (likely(!IS_NULL(cnd))) {
    X(cnd);
    return lisp_prog(closure, prg, UP(NIL));
  }
  /*
   * Or return NIL;
   */
  X(cnd, prg);
  return UP(NIL);
}

LISP_PLUGIN_REGISTER(when, when, COND, REM)

// vim: tw=80:sw=2:ts=2:sts=2:et
