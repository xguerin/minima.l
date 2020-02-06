#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_function_lambda(const lisp_t lisp, const atom_t closure,
                     const atom_t arguments)
{
  LISP_LOOKUP(cell, arguments, @);
  /*
   * Extract the arguments and the PROG of the lambda.
   */
  atom_t args = lisp_car(cell);
  atom_t prog = lisp_cdr(cell);
  X(cell);
  /*
   * Append an empty currying list and capture the closure.
   */
  atom_t con0 = lisp_cons(NIL, prog);
  atom_t con1 = lisp_cons(closure, con0);
  atom_t con2 = lisp_cons(args, con1);
  X(prog, args, con0, con1);
  /*
   * Return the lambda.
   */
  return con2;
}

LISP_PLUGIN_REGISTER(lambda, \\, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
