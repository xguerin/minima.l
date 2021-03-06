#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_prin_all(const lisp_t lisp, const atom_t closure, const atom_t cell,
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
   * Print CAR and recurse on CDR.
   */
  lisp_prin(lisp, car, false);
  return lisp_prin_all(lisp, closure, cdr, car);
}

static atom_t USED
lisp_function_prin(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, ANY);
  return lisp_prin_all(lisp, C, UP(ANY), lisp_make_nil(lisp));
}

LISP_MODULE_SETUP(prin, prin, ANY);

// vim: tw=80:sw=2:ts=2:sts=2:et
