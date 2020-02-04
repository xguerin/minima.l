#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_function_eval(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(car, arguments, X);
  return lisp_eval(closure, car);
}

LISP_PLUGIN_REGISTER(eval, eval, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
