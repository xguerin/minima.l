#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <string.h>

static atom_t
lisp_prin_all(const atom_t closure, const atom_t cell, const atom_t result)
{
  if (unlikely(IS_NULL(cell))) {
    X(cell);
    return result;
  }
  /*
   */
  atom_t car = lisp_eval(closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  lisp_prin(closure, car, false);
  X(cell); X(result);
  return lisp_prin_all(closure, cdr, car);
}

atom_t
lisp_function_prin(const atom_t closure, const atom_t cell)
{
  return lisp_prin_all(closure, cell, UP(NIL));
}

LISP_REGISTER(prin, prin)
