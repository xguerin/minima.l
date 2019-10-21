#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_stream(const atom_t closure, const atom_t cell, const atom_t result)
{
  TRACE_SEXP(cell);
  TRACE_SEXP(result);
  /*
   */
  if (likely(IS_PAIR(cell))) {
    TRACE_SEXP(cell);
    /*
     * Get CAR/CDR.
     */
    atom_t car = lisp_car(cell);
    atom_t cdr = lisp_cdr(cell);
    X(cell);
    /*
     * Bind the result.
     */
    atom_t con = lisp_cons(car, result);
    X(car, result);
    atom_t nxt = lisp_cons(con, NIL);
    X(con);
    /*
     * Process the rest of the arguments.
     */
    return lisp_stream(closure, cdr, nxt);
  }
  /*
   * Evaluate the result.
   */
  TRACE_SEXP(result);
  if (likely(IS_PAIR(result))) {
    atom_t car = lisp_car(result);
    X(cell, result);
    TRACE_SEXP(car);
    return lisp_eval(closure, car);
  }
  /*
   */
  return result;
}

static atom_t
lisp_function_stream(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(cell, arguments, @);
  /*
   * Grab CAR/CDR.
   */
  atom_t car = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  atom_t con = lisp_cons(car, NIL);
  X(car, cell);
  /*
   */
  return lisp_stream(closure, cdr, con);
}

LISP_PLUGIN_REGISTER(stream, |>, @)
