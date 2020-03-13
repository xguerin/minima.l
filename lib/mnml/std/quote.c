#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_quote(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, cell, closure, @);
  return cell;
}

LISP_MODULE_SETUP(quote, quote, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
