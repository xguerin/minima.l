#include <mnml/cps.h>
#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/maker.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

/*
 * CPS entry point.
 */

static atom_t
lisp_function_lift(const atom_t closure, const atom_t cell)
{
  TRACE_SEXP(cell);
  /*
   * Evaluate the symbol argument.
   */
  atom_t fun = lisp_eval(closure, lisp_car(cell));
  X(cell);
  /*
   * Now, extract the argument list, the closure and the body.
   */
  SPLIT_FUNCTION(fun, args, clos, body);
  X(args); X(clos);
  /*
   * Wrap all funcalls with lambda wrappers.
   */
  return lisp_cps_lift(body, 0);
}

LISP_PLUGIN_REGISTER(lift, lift)
