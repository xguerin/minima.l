#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_function_conc(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(fst, arguments, X);
  LISP_LOOKUP(snd, arguments, Y);
  atom_t res = lisp_conc(fst, snd);
  X(fst, snd);
  return res;
}

LISP_PLUGIN_REGISTER(conc, conc, X, Y, NIL)
