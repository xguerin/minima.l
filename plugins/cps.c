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
lisp_function_cps(const atom_t closure, const atom_t cell)
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
  /*
   * Wrap all funcalls with lambda wrappers.
   */
  atom_t llist = lisp_cps_convert(body, 0);
  atom_t lambd = lisp_cps_bind(llist);
  /*
   * Grab the lambda's arg and body.
   */
  SPLIT_LAMBDA(lambd, symb, larg, lbdy);
  X(symb);
  /*
   * Stitch them to the original args.
   */
  atom_t dupl = lisp_dup(args);
  atom_t next = lisp_conc(dupl, larg);
  X(args); X(dupl); X(larg);
  /*
   * Reconstruct the function call.
   */
  MAKE_FUNCTION(result, next, clos, lbdy);
  return result;
}

LISP_PLUGIN_REGISTER(cps, >&)
