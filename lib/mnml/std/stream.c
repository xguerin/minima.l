#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_stream(const lisp_t lisp, const atom_t closure, const atom_t ANY,
            const atom_t expr)
{
  /*
   * Evaluate the result.
   */
  atom_t res = lisp_eval(lisp, closure, expr);
  /*
   * Return the evaluated result if CELL is NIL.
   */
  if (IS_NULL(ANY)) {
    X(lisp, ANY);
    return res;
  }
  /*
   * Grab CAR and CDR.
   */
  atom_t car = lisp_car(lisp, ANY);
  atom_t cdr = lisp_cdr(lisp, ANY);
  X(lisp, ANY);
  /*
   * Bind the quoted result.
   */
  atom_t cn0 = lisp_cons(lisp, lisp_make_quote(lisp), res);
  atom_t cn1 = lisp_cons(lisp, cn0, lisp_make_nil(lisp));
  atom_t nxt = lisp_cons(lisp, car, cn1);
  /*
   * Call recursively.
   */
  return lisp_stream(lisp, closure, cdr, nxt);
}

static atom_t USED
lisp_function_stream(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, ANY);
  /*
   * Grab CAR/CDR.
   */
  atom_t car = lisp_car(lisp, ANY);
  atom_t cdr = lisp_cdr(lisp, ANY);
  /*
   */
  return lisp_stream(lisp, C, cdr, car);
}

/* clang-format off */
LISP_MODULE_SETUP(stream, |>, ANY)
/* clang-format */

// vim: tw=80:sw=2:ts=2:sts=2:et
