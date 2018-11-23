#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

static atom_t
lisp_function_cons(const atom_t closure, const atom_t cell)
{
  atom_t fst = lisp_eval(closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  atom_t snd = lisp_eval(closure, lisp_car(cdr));
  X(cdr);
  atom_t res = lisp_cons(fst, snd);
  X(fst); X(snd);
  return res;
}

LISP_PLUGIN_REGISTER(cons, cons)
