#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_prog(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, ANY);
  return lisp_prog(lisp, C, UP(ANY), lisp_make_nil(lisp));
}

LISP_MODULE_SETUP(prog, prog, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
