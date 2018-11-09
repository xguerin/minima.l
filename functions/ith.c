#include "utils.h"
#include <lisp/lisp.h>
#include <lisp/slab.h>
#include <string.h>

static atom_t
lisp_function_ith(const atom_t closure, const atom_t cell)
{
  /*
   * Evaluate the condition expression.
   */
  atom_t cnd = lisp_eval(closure, lisp_car(cell));
  atom_type_t type = cnd->type;
  X(cnd);
  /*
   * Get the THEN branch.
   */
  atom_t cd0 = lisp_cdr(cell);
  X(cell);
  atom_t thn = lisp_car(cd0);
  X(cd0);
  /*
   * Execute the THEN branch.
   */
  if (type == T_TRUE) {
    return lisp_eval(closure, thn);
  }
  /*
   * Clean-up.
   */
  X(thn);
  return UP(NIL);
}

LISP_REGISTER(ith, ?)
