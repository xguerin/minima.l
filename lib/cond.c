#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_cond(const atom_t closure, const atom_t cell, const atom_t match)
{
  TRACE_SEXP(cell);
  /*
   * Sanity checks.
   */
  if (IS_NULL(match) || !IS_PAIR(match) || !IS_PAIR(CAR(match))) {
    X(cell, match);
    return UP(NIL);
  }
  /*
   * Get CAR/CDR.
   */
  atom_t car = lisp_car(match);
  atom_t cdr = lisp_cdr(match);
  X(match);
  /*
   * Get args and prog.
   */
  atom_t args = lisp_car(car);
  atom_t prog = lisp_cdr(car);
  X(car);
  /*
   * If the cond argument _, simply execute the program.
   */
  if (IS_WILD(args)) {
    X(args, cdr, cell);
    return lisp_eval(closure, prog);
  }
  /*
   * Evaluate the predicate.
   */
  atom_t con = lisp_cons(cell, NIL);
  atom_t evl = lisp_cons(args, con);
  atom_t res = lisp_eval(closure, evl);
  X(con, res, args);
  /*
   */
  if (IS_TRUE(res)) {
    X(cdr, cell);
    return lisp_eval(closure, prog);
  }
  /*
   */
  X(prog);
  return lisp_cond(closure, cell, cdr);
}

static atom_t
lisp_function_cond(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(cell, arguments, @);
  atom_t car = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  return lisp_cond(closure, car, cdr);
}

LISP_PLUGIN_REGISTER(cond, cond, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
