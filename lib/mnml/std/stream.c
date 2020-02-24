#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_stream(const lisp_t lisp, const atom_t closure, const atom_t cell,
            const atom_t expr)
{
  TRACE_SEXP(cell);
  TRACE_SEXP(expr);
  /*
   * Evaluate the result.
   */
  atom_t res = lisp_eval(lisp, closure, expr);
  /*
   * Return the evaluated result if CELL is NIL.
   */
  if (IS_NULL(cell)) {
    X(cell);
    return res;
  }
  /*
   * Grab CAR and CDR.
   */
  atom_t car = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  /*
   * Bind the quoted result.
   */
  atom_t cn0 = lisp_cons(QUOTE, res);
  atom_t cn1 = lisp_cons(cn0, NIL);
  atom_t nxt = lisp_cons(car, cn1);
  X(res, car, cn0, cn1);
  /*
   * Call recursively.
   */
  return lisp_stream(lisp, closure, cdr, nxt);
}

static atom_t
lisp_function_stream(const lisp_t lisp, const atom_t closure,
                     const atom_t arguments)
{
  LISP_LOOKUP(cell, arguments, @);
  /*
   * Grab CAR/CDR.
   */
  atom_t car = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  /*
   */
  return lisp_stream(lisp, closure, cdr, car);
}

/* clang-format off */
LISP_MODULE_SETUP(stream, |>, @)
/* clang-format */

// vim: tw=80:sw=2:ts=2:sts=2:et
