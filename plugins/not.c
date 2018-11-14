#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

static atom_t
lisp_function_not(const atom_t closure, const atom_t cell)
{
  atom_t car = lisp_eval(closure, lisp_car(cell));
  X(cell);
  atom_t res = IS_NULL(car) ? TRUE : NIL;
  X(car);
  return UP(res);
}

LISP_REGISTER(not, not)
