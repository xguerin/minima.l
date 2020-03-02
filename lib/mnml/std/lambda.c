#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_function_lambda(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, cell, closure, @);
  /*
   * Extract the arguments and the PROG of the lambda.
   */
  atom_t args = lisp_car(cell);
  atom_t prog = lisp_cdr(cell);
  X(cell);
  /*
   * Append an empty currying list and capture the closure.
   */
  atom_t con0 = lisp_cons(closure, prog);
  atom_t con1 = lisp_cons(args, con0);
  X(prog, args, con0);
  /*
   * Return the lambda.
   */
  return con1;
}

LISP_MODULE_SETUP(lambda, \\, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
