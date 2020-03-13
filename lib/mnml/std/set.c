#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_set(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, cell, closure, @);
  /*
   * Lookup the symbol and the value.
   */
  atom_t sym = lisp_eval(lisp, closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  atom_t val = lisp_eval(lisp, closure, lisp_car(cdr));
  X(cdr, cell);
  /*
   * Check if sym is a symbol.
   */
  if (unlikely(!IS_SYMB(sym))) {
    X(sym, val);
    return UP(NIL);
  }
  /*
   * Lookup first in the closure.
   */
  FOREACH(closure, c0) {
    atom_t car = c0->car;
    if (lisp_symbol_match(CAR(car), &sym->symbol)) {
      atom_t res = CDR(car);
      CDR(car) = val;
      X(sym);
      return res;
    }
    NEXT(c0);
  }
  /*
   * Look second in globals.
   */
  FOREACH(GLOBALS, c1)
  {
    atom_t car = c1->car;
    if (lisp_symbol_match(CAR(car), &sym->symbol)) {
      atom_t res = CDR(car);
      CDR(car) = val;
      X(sym);
      return res;
    }
    NEXT(c1);
  }
  /*
   * Set the symbol in the closure stack.
   */
  X(sym, val);
  return UP(NIL);
}

/* clang-format off */
LISP_MODULE_SETUP(set, <-, @)
/* clang-format */

// vim: tw=80:sw=2:ts=2:sts=2:et
