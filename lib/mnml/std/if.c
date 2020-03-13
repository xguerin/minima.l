#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_if(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, cnd, closure, COND);
  LISP_LOOKUP(lisp, ops, closure, REM);
  /*
   * Evaluate the THEN branch if TRUE.
   */
  if (unlikely(IS_NULL(cnd))) {
    atom_t cd1 = lisp_cdr(ops);
    atom_t els = lisp_car(cd1);
    atom_t res = lisp_eval(lisp, closure, els);
    X(cnd, ops, cd1);
    return res;
  }
  /*
   * Or evaluate the ELSE branch.
   */
  else {
    atom_t thn = lisp_car(ops);
    atom_t res = lisp_eval(lisp, closure, thn);
    X(cnd, ops);
    return res;
  }
}

LISP_MODULE_SETUP(if, if, COND, REM)

// vim: tw=80:sw=2:ts=2:sts=2:et
