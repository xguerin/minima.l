#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_function_eval(const lisp_t lisp, const atom_t closure,
                   const atom_t arguments)
{
  LISP_LOOKUP(car, arguments, X);
  return lisp_eval(lisp, closure, car);
}

LISP_MODULE_SETUP(eval, eval, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
