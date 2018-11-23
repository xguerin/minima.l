#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

static atom_t
lisp_function_cdr(const atom_t closure, const atom_t cell)
{
  atom_t arg = lisp_eval(closure, lisp_car(cell));
  X(cell);
  atom_t res = lisp_cdr(arg);
  X(arg);
  return res;
}

LISP_PLUGIN_REGISTER(cdr, cdr)
