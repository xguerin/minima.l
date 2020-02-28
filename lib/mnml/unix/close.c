#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <unistd.h>

static bool
lisp_close(const lisp_t lisp, const atom_t closure, const atom_t cell,
           const bool res)
{
  if (!IS_NULL(cell)) {
    atom_t car = lisp_eval(lisp, closure, lisp_car(cell));
    atom_t cdr = lisp_cdr(cell);
    int s = close(car->number);
    X(car, cell);
    return lisp_close(lisp, closure, cdr, res && s == 0);
  }
  /*
   */
  X(cell);
  return res;
}

static atom_t
lisp_function_close(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(cell, closure, @);
  bool res = lisp_close(lisp, closure, cell, true);
  return UP(res ? TRUE : NIL);
}

LISP_MODULE_SETUP(close, close, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
