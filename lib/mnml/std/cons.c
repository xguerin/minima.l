#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_function_cons(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(fst, closure, X);
  LISP_LOOKUP(snd, closure, Y);
  atom_t res = lisp_cons(fst, snd);
  X(fst, snd);
  return res;
}

LISP_MODULE_SETUP(cons, cons, X, Y, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
