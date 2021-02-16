#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_neq(UNUSED const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, X, Y);
  atom_t res = lisp_neq(X, Y) ? TRUE : NIL;
  return UP(res);
}

LISP_MODULE_SETUP(neq, <>, X, Y, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
