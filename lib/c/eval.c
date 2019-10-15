#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

static atom_t
lisp_function_eval(const atom_t closure, const atom_t cell)
{
  atom_t car = lisp_eval(closure, lisp_car(cell));
  X(cell);
  return lisp_eval(closure, car);
}

LISP_PLUGIN_REGISTER(eval, eval)
