#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <limits.h>
#include <unistd.h>

static atom_t USED
lisp_function_unlink(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, NAME);
  /*
   * Make sure the argument is a string.
   */
  if (!lisp_is_string(NAME)) {
    return lisp_make_nil(lisp);
  }
  /*
   * Convert the argument.
   */
  char name_buf[PATH_MAX];
  lisp_make_cstring(NAME, name_buf, PATH_MAX, 0);
  /*
   * Call unlink().
   */
  return unlink(name_buf) == 0 ? lisp_make_true(lisp) : lisp_make_nil(lisp);
}

LISP_MODULE_SETUP(unlink, unlink, NAME, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
