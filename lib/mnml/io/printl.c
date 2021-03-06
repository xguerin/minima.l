#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <stdio.h>

static atom_t
lisp_printl_all(const lisp_t lisp, const atom_t closure, const atom_t cell,
                const atom_t result)
{
  /*
   * Return the result if we are done.
   */
  if (unlikely(IS_NULL(cell))) {
    X(lisp, cell);
    return result;
  }
  X(lisp, result);
  /*
   * Grab CAR and CDR, and evaluate CAR.
   */
  atom_t car = lisp_eval(lisp, closure, lisp_car(lisp, cell));
  atom_t cdr = lisp_cdr(lisp, cell);
  X(lisp, cell);
  /*
   * Print the evaluated CAR and recurse on CDR.
   */
  lisp_prin(lisp, car, true);
  if (!IS_NULL(cdr)) {
    fwrite(" ", 1, 1, (FILE*)CAR(CAR(lisp->ochan))->number);
  }
  return lisp_printl_all(lisp, closure, cdr, car);
}

static atom_t USED
lisp_function_printl(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, ANY);
  atom_t res = lisp_printl_all(lisp, C, UP(ANY), lisp_make_nil(lisp));
  fwrite("\n", 1, 1, (FILE*)CAR(CAR(lisp->ochan))->number);
  return res;
}

LISP_MODULE_SETUP(printl, printl, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
