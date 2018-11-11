#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <string.h>

atom_t
lisp_function_prog(const atom_t closure, const atom_t cell)
{
  return lisp_prog(closure, cell, UP(NIL));
}

LISP_REGISTER(prog, prog)
