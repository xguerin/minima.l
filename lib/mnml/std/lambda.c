#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_lambda(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, ANY);
  /*
   * Extract the arguments and the PROG of the lambda.
   */
  atom_t args = lisp_car(lisp, ANY);
  atom_t prog = lisp_cdr(lisp, ANY);
  /*
   * Append an empty currying list, capture the closure, and return the lambda.
   */
  atom_t clos = lisp_dup(lisp, C);
  atom_t con0 = lisp_cons(lisp, clos, prog);
  return lisp_cons(lisp, args, con0);
}

LISP_MODULE_SETUP(lambda, \\, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
