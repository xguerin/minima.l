#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_or(UNUSED const lisp_t lisp, const atom_t closure)
{
  TRACE_CLOS_SEXP(closure);
  LISP_ARGS(closure, C, ANY);
  /*
   * Grab the two arguments.
   */
  const atom_t ar0 = lisp_car(lisp, ANY);
  const atom_t cdr = lisp_cdr(lisp, ANY);
  const atom_t ar1 = lisp_car(lisp, cdr);
  X(lisp, cdr);
  /*
   * Evaluate the first argument, early exit if not NIL.
   */
  const atom_t ra0 = lisp_eval(lisp, C, ar0);
  if (!IS_NULL(ra0)) {
    X(lisp, ra0, ar1);
    return lisp_make_true(lisp);
  }
  X(lisp, ra0);
  /*
   * Evaluate the second argument.
   */
  const atom_t ra1 = lisp_eval(lisp, C, ar1);
  if (IS_NULL(ra1)) {
    return ra1;
  }
  /*
   * Return.
   */
  X(lisp, ra1);
  return lisp_make_true(lisp);
}

LISP_MODULE_SETUP(or, or, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
