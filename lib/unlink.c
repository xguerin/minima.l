#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <limits.h>
#include <unistd.h>

static atom_t
lisp_function_unlink(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(name, arguments, NAME);
  /*
   * Make sure the argument is a string.
   */
  if (!lisp_is_string(name)) {
    X(name);
    return UP(NIL);
  }
  /*
   * Convert the argument.
   */
  char name_buf[PATH_MAX];
  lisp_make_cstring(name, name_buf, PATH_MAX, 0);
  X(name);
  /*
   * Call unlink().
   */
  return unlink(name_buf) == 0 ? UP(TRUE) : UP(NIL);
}

LISP_PLUGIN_REGISTER(unlink, unlink, NAME, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
