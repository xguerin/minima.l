#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <unistd.h>

static bool
lisp_close(const atom_t closure, const atom_t cell, const bool res)
{
  if (!IS_NULL(cell)) {
    atom_t car = lisp_eval(closure, lisp_car(cell));
    atom_t cdr = lisp_cdr(cell);
    int s = close(car->number);
    X(car, cell);
    return lisp_close(closure, cdr, res && s == 0);
  }
  /*
   */
  X(cell);
  return res;
}

static atom_t
lisp_function_close(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(cell, arguments, @);
  bool res = lisp_close(closure, cell, true);
  return UP(res ? TRUE : NIL);
}

LISP_PLUGIN_REGISTER(close, close, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
