#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t USED
lisp_function_equ(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, X, Y);
  return lisp_equ(X, Y) ? lisp_make_true(lisp) : lisp_make_nil(lisp);
}

LISP_MODULE_SETUP(equ, =, X, Y, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
