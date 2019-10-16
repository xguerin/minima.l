#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

atom_t
lisp_function_read(const atom_t closure, const atom_t cell)
{
  atom_t result = lisp_read(closure, cell);
  return result == NULL ? UP(NIL) : result;
}

LISP_PLUGIN_REGISTER(read, read)
