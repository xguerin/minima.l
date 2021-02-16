#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t USED
lisp_function_equ(UNUSED const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, X, Y);
  atom_t res = lisp_equ(X, Y) ? TRUE : NIL;
  return UP(res);
}

LISP_MODULE_SETUP(equ, =, X, Y, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
