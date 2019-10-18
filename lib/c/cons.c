#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_function_cons(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(fst, arguments, X);
  LISP_LOOKUP(snd, arguments, Y);
  atom_t res = lisp_cons(fst, snd);
  X(fst); X(snd);
  return res;
}

LISP_PLUGIN_REGISTER(cons, cons, X, Y, NIL)
