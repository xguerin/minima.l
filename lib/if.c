#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_function_if(const lisp_t lisp, const atom_t closure,
                 const atom_t arguments)
{
  LISP_LOOKUP(cnd, arguments, COND);
  LISP_LOOKUP(ops, arguments, REM);
  /*
   * Evaluate the THEN branch if TRUE.
   */
  if (unlikely(IS_NULL(cnd))) {
    atom_t cd1 = lisp_cdr(ops);
    atom_t els = lisp_car(cd1);
    X(cnd, ops, cd1);
    return lisp_eval(lisp, closure, els);
  }
  /*
   * Or evaluate the ELSE branch.
   */
  else {
    atom_t thn = lisp_car(ops);
    X(cnd, ops);
    return lisp_eval(lisp, closure, thn);
  }
}

LISP_PLUGIN_REGISTER(if, if, COND, REM)

// vim: tw=80:sw=2:ts=2:sts=2:et
