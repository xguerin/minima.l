#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_function_prog(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(cell, arguments, @);
  return lisp_prog(closure, cell, UP(NIL));
}

LISP_PLUGIN_REGISTER(prog, prog, @)
