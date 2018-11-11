#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <string.h>

static atom_t
lisp_function_eval(const atom_t closure, const atom_t cell)
{
  atom_t car = lisp_eval(closure, lisp_car(cell));
  X(cell);
  return lisp_eval(closure, car);
}

LISP_REGISTER(eval, eval)
