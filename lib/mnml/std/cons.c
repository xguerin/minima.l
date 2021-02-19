#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_cons(UNUSED const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, X, Y);
  return lisp_cons(UP(X), UP(Y));
}

LISP_MODULE_SETUP(cons, cons, X, Y, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
