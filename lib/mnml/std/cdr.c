#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_cdr(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, arg, closure, X);
  atom_t res = lisp_cdr(arg);
  X(arg);
  return res;
}

LISP_MODULE_SETUP(cdr, cdr, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
