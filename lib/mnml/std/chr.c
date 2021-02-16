#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t USED
lisp_function_chr(UNUSED const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, X);
  char val = X->number;
  return lisp_make_char(val);
}

LISP_MODULE_SETUP(chr, chr, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
