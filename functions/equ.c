#include "utils.h"
#include <lisp/lisp.h>
#include <lisp/slab.h>
#include <string.h>

static atom_t
lisp_function_equ(const atom_t closure, const atom_t cell)
{
  atom_t vl0 = lisp_eval(closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  atom_t vl1 = lisp_eval(closure, lisp_car(cdr));
  X(cdr);
  atom_t res = lisp_equl(vl0, vl1) ? TRUE : NIL;
  X(vl0); X(vl1);
  return UP(res);
}

LISP_REGISTER(equ, =)
