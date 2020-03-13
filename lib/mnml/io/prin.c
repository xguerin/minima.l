#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_prin_all(const lisp_t lisp, const atom_t closure, const atom_t cell,
              const atom_t result)
{
  if (unlikely(IS_NULL(cell))) {
    X(cell);
    return result;
  }
  /*
   */
  atom_t car = lisp_eval(lisp, closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  lisp_prin(lisp, closure, car, false);
  X(cell, result);
  return lisp_prin_all(lisp, closure, cdr, car);
}

static atom_t USED
lisp_function_prin(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, cell, closure, @);
  return lisp_prin_all(lisp, closure, cell, UP(NIL));
}

LISP_MODULE_SETUP(prin, prin, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
