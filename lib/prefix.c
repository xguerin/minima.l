#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_function_prefix(const atom_t closure, const atom_t arguments)
{
  const char * const prefix = lisp_prefix();
  return lisp_make_string(prefix, strlen(prefix));
}

LISP_PLUGIN_REGISTER(prefix, prefix)

// vim: tw=80:sw=2:ts=2:sts=2:et
