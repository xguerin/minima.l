#include "utils.h"
#include <lisp/lisp.h>
#include <lisp/slab.h>
#include <string.h>

static atom_t
lisp_function_setq(const atom_t closure, const atom_t cell)
{
  atom_t sym = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  atom_t val = lisp_eval(closure, lisp_car(cdr));
  X(cdr);
  GLOBALS = lisp_setq(GLOBALS, sym, UP(val));
  return val;
}

LISP_REGISTER(setq, setq)
