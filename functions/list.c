#include "utils.h"
#include <lisp/lisp.h>
#include <lisp/slab.h>
#include <string.h>

static atom_t
lisp_function_list(const atom_t closure, const atom_t cell)
{
  return lisp_list(closure, cell);
}

LISP_REGISTER(list, ::)
