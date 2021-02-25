#include "mnml/maker.h"
#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static size_t
to_string(const atom_t cell, char* const buffer)
{
  /*
   * Check that the first argument is a string.
   */
  if (unlikely(!(IS_PAIR(cell) && lisp_is_string(cell)))) {
    return 0;
  }
  /*
   * Process the string.
   */
  return lisp_make_cstring(cell, buffer, LISP_SYMBOL_LENGTH, 0);
}

static atom_t USED
lisp_function_sym(const lisp_t lisp, const atom_t closure)
{
  atom_t res;
  LISP_ARGS(closure, C, ANY);
  /*
   * Grab the CAR and CDR.
   */
  atom_t el0 = lisp_eval(lisp, closure, lisp_car(lisp, ANY));
  atom_t cdr = lisp_cdr(lisp, ANY);
  if (IS_NULL(el0)) {
    X(lisp->slab, cdr);
    return el0;
  }
  /*
   * Convert the first argument.
   */
  char str0[17] = { 0 };
  size_t len0 = to_string(el0, str0);
  if (len0 == 0) {
    X(lisp->slab, el0, cdr);
    return lisp_make_nil(lisp);
  }
  X(lisp->slab, el0);
  /*
   * Check what to do based on how many arguments remain.
   */
  switch (lisp_len(cdr)) {
    case 0: {
      MAKE_SYMBOL_STATIC(symb, str0, len0);
      res = lisp_make_symbol(lisp, symb);
      X(lisp->slab, cdr);
      break;
    }
    case 1: {
      /*
       * Grab the CAR.
       */
      atom_t el1 = lisp_eval(lisp, closure, lisp_car(lisp, cdr));
      X(lisp->slab, cdr);
      /*
       * Convert the second argument.
       */
      char str1[17] = { 0 };
      size_t len1 = to_string(el1, str1);
      if (len1 == 0) {
        X(lisp->slab, el0, el1);
        res = lisp_make_nil(lisp);
        break;
      }
      X(lisp->slab, el1);
      /*
       * Make the scoped symbol.
       */
      MAKE_SCOPED_SYMBOL_STATIC(symb, str0, len0, str1, len1);
      res = lisp_make_scoped_symbol(lisp, symb);
      break;
    }
    default: {
      X(lisp->slab, cdr);
      return lisp_make_nil(lisp);
    }
  }
  /*
   * Return the result.
   */
  return res;
}

LISP_MODULE_SETUP(sym, sym, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
