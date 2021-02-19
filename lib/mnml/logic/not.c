#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_function_not(UNUSED const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, X);
  return IS_NULL(X) ? lisp_make_true() : lisp_make_nil();
}

LISP_MODULE_SETUP(not, not, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
