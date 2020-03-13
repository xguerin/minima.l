#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_prog(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, cell, closure, @);
  return lisp_prog(lisp, closure, cell, UP(NIL));
}

LISP_MODULE_SETUP(prog, prog, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
