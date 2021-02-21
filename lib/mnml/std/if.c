#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_if(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, COND, REM);
  /*
   * Evaluate the THEN branch if TRUE.
   */
  if (unlikely(IS_NULL(COND))) {
    atom_t cd1 = lisp_cdr(REM);
    atom_t els = lisp_car(cd1);
    X(cd1);
    return lisp_eval(lisp, C, els);
  }
  /*
   * Or evaluate the ELSE branch.
   */
  else {
    atom_t thn = lisp_car(REM);
    return lisp_eval(lisp, C, thn);
  }
}

LISP_MODULE_SETUP(if, if, COND, REM)

// vim: tw=80:sw=2:ts=2:sts=2:et
