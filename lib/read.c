#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_function_read(const atom_t closure, const atom_t arguments)
{
  atom_t result = lisp_read(closure, UP(NIL));
  return result == NULL ? UP(NIL) : result;
}

LISP_PLUGIN_REGISTER(read, read)
