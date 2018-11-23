#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

atom_t
lisp_function_read(const atom_t closure, const atom_t cell)
{
  return lisp_read(closure, cell);
}

LISP_PLUGIN_REGISTER(read, read)
