#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_neq(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, vl0, closure, X);
  LISP_LOOKUP(lisp, vl1, closure, Y);
  atom_t res = lisp_neq(vl0, vl1) ? TRUE : NIL;
  X(vl0, vl1);
  return UP(res);
}

LISP_MODULE_SETUP(neq, <>, X, Y, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
