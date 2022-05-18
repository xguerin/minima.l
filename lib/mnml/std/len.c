#include <mnml/types.h>
#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t USED
lisp_function_len(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, X);
  if (IS_LIST(X)) {
    return lisp_make_number(lisp, (int64_t)lisp_len(X));
  }
  return lisp_make_nil(lisp);
}

LISP_MODULE_SETUP(len, len, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
