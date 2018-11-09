#include "utils.h"
#include <lisp/lisp.h>
#include <lisp/slab.h>
#include <string.h>

static atom_t
lisp_function_def(const atom_t closure, const atom_t cell)
{
  atom_t sym = lisp_car(cell);
  atom_t val = lisp_cdr(cell);
  GLOBALS = lisp_setq(GLOBALS, sym, UP(val));
  X(cell);
  return val;
}

LISP_REGISTER(def, def)
