#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_function_lambda(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(cell, arguments, @);
  /*
   * Extract the arguments and the PROG of the lambda.
   */
  atom_t args = lisp_car(cell);
  atom_t prog = lisp_cdr(cell);
  X(cell);
  /*
   * Embed the closure in the definition.
   */
  atom_t con0 = lisp_cons(closure, prog);
  atom_t con1 = lisp_cons(args, con0);
  X(prog, args, con0);
  /*
   * Return the lambda.
   */
  return con1;
}

LISP_PLUGIN_REGISTER(lambda, \\, @)
