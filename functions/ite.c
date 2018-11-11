#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <string.h>

static atom_t
lisp_function_ite(const atom_t closure, const atom_t cell)
{
  /*
   * Evaluate the condition expression.
   */
  atom_t cnd = lisp_eval(closure, lisp_car(cell));
  atom_type_t type = cnd->type;
  X(cnd);
  atom_t cd0 = lisp_cdr(cell);
  X(cell);
  /*
   * Evaluate the THEN branch if TRUE.
   */
  if (type == T_TRUE) {
    atom_t thn = lisp_car(cd0);
    X(cd0);
    return lisp_eval(closure, thn);
  }
  /*
   * Or evaluate the ELSE branch.
   */
  else {
    atom_t cd1 = lisp_cdr(cd0);
    X(cd0);
    atom_t els = lisp_car(cd1);
    X(cd1);
    return lisp_eval(closure, els);
  }
}

LISP_REGISTER(ite, ?:)
