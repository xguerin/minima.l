#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <string.h>

static atom_t
lisp_function_car(const atom_t closure, const atom_t cell)
{
  atom_t arg = lisp_eval(closure, lisp_car(cell));
  X(cell);
  atom_t res = lisp_car(arg);
  X(arg);
  return res;
}

LISP_REGISTER(car, car)
