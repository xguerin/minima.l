#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_function_not(const lisp_t lisp, const atom_t closure,
                  const atom_t arguments)
{
  LISP_LOOKUP(car, arguments, X);
  atom_t res = IS_NULL(car) ? TRUE : NIL;
  X(car);
  return UP(res);
}

LISP_MODULE_SETUP(not, not, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
