#include <mnml/debug.h>
#include <mnml/maker.h>
#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <fcntl.h>
#include <limits.h>

static atom_t
lisp_ns_get_all(const lisp_t lisp, const atom_t cell)
{
  if (IS_NULL(cell)) {
    return lisp_make_nil(lisp);
  }
  /*
   * Grap the symbol of the head scope.
   */
  atom_t sym = UP(CAR(CAR(cell)));
  return lisp_cons(lisp, sym, lisp_ns_get_all(lisp, CDR(cell)));
}

static atom_t
lisp_ns_get(const lisp_t lisp, const atom_t cell, const atom_t symb)
{
  if (IS_NULL(cell)) {
    X(lisp->slab, symb);
    return lisp_make_nil(lisp);
  }
  /*
   * If the symbols match, return the content.
   */
  if (lisp_symbol_match(CAR(CAR(cell)), &symb->symbol)) {
    X(lisp->slab, symb);
    return lisp_cdr(lisp, CAR(cell));
  }
  /*
   * Else, keep looking.
   */
  return lisp_ns_get(lisp, CDR(cell), symb);
}

static atom_t
lisp_function_ns(const lisp_t lisp, const atom_t closure)
{
  TRACE_CLOS_SEXP(closure);
  LISP_ARGS(closure, C, ANY);
  /*
   * Grab and evaluate the CAR.
   */
  atom_t car = lisp_eval(lisp, closure, lisp_car(lisp, ANY));
  /*
   * Check if CAR has the proper type.
   */
  if (!IS_NULL(car) && !IS_SYMB(car)) {
    X(lisp->slab, car);
    return lisp_make_nil(lisp);
  }
  /*
   * If CAR is NIL, return all the available scopes.
   */
  if (IS_NULL(car)) {
    X(lisp->slab, car);
    return lisp_ns_get_all(lisp, lisp->scopes);
  }
  /*
   * If it's a symbol, return the scope matching that symbol.
   */
  return lisp_ns_get(lisp, lisp->scopes, car);
}

LISP_MODULE_SETUP(ns, ns, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
