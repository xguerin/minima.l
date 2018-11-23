#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

static bool
lisp_close(const atom_t closure, const atom_t cell, const bool res)
{
  if (!IS_NULL(cell)) {
    atom_t car = lisp_eval(closure, lisp_car(cell));
    atom_t cdr = lisp_cdr(cell);
    int s = close(car->number);
    X(car); X(cell);
    return lisp_close(closure, cdr, res && s == 0);
  }
  /*
   */
  X(cell);
  return res;
}

atom_t
lisp_function_close(const atom_t closure, const atom_t cell)
{
  bool res = lisp_close(closure, cell, true);
  return UP(res ? TRUE : NIL);
}

LISP_PLUGIN_REGISTER(close, close)
