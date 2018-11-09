#include "utils.h"
#include <lisp/lisp.h>
#include <lisp/slab.h>
#include <string.h>

static atom_t
lisp_function_pipe(const atom_t closure, const atom_t cell)
{
  return lisp_pipe(closure, cell, UP(NIL));
}

LISP_REGISTER(pipe, |)
