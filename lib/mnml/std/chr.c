#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t USED
lisp_function_chr(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, X);
  char val = (char)X->number;
  return lisp_make_char(lisp, val);
}

LISP_MODULE_SETUP(chr, chr, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
