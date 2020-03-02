#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_function_car(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, arg, closure, X);
  atom_t res = lisp_car(arg);
  X(arg);
  return res;
}

LISP_MODULE_SETUP(car, car, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
