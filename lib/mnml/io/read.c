#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_read(const lisp_t lisp, const atom_t closure)
{
  atom_t result = lisp_read(lisp, closure, lisp_make_nil(lisp));
  return result == NULL ? lisp_make_nil(lisp) : result;
}

LISP_MODULE_SETUP(read, read)

// vim: tw=80:sw=2:ts=2:sts=2:et
