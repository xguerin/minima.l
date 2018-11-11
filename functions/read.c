#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <string.h>

atom_t
lisp_function_read(const atom_t closure, const atom_t cell)
{
  return lisp_read(closure, cell);
}

LISP_REGISTER(read, read)
