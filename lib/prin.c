#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

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
  X(cell, result);
  return lisp_prin_all(closure, cdr, car);
}

static atom_t
lisp_function_prin(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(cell, arguments, @);
  return lisp_prin_all(closure, cell, UP(NIL));
}

LISP_PLUGIN_REGISTER(prin, prin, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
