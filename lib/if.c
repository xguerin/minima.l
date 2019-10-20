#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_function_if(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(cnd, arguments, COND);
  LISP_LOOKUP(ops, arguments, PROG);
  /*
   * Evaluate the THEN branch if TRUE.
   */
  if (unlikely(IS_NULL(cnd))) {
    atom_t cd1 = lisp_cdr(ops);
    atom_t els = lisp_car(cd1);
    X(cnd); X(ops); X(cd1);
    return lisp_eval(closure, els);
  }
  /*
   * Or evaluate the ELSE branch.
   */
  else {
    atom_t thn = lisp_car(ops);
    X(cnd); X(ops);
    return lisp_eval(closure, thn);
  }
}

LISP_PLUGIN_REGISTER(if, if, COND, PROG)
