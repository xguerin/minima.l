#include "utils.h"
#include <lisp/lisp.h>
#include <lisp/slab.h>
#include <string.h>

atom_t
lisp_function_quote(const atom_t closure, const atom_t cell)
{
  return cell;
}

LISP_REGISTER(quote, quote)
