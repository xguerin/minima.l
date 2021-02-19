#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_set(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, ANY);
  /*
   * Lookup the symbol and the value.
   */
  atom_t sym = lisp_eval(lisp, C, lisp_car(ANY));
  atom_t cdr = lisp_cdr(ANY);
  atom_t val = lisp_eval(lisp, C, lisp_car(cdr));
  /*
   * Check if sym is a symbol.
   */
  if (unlikely(!IS_SYMB(sym))) {
    X(sym, val);
    return lisp_make_nil();
  }
  /*
   * Lookup first in the closure.
   */
  FOREACH(C, c0)
  {
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
  return lisp_make_nil();
}

/* clang-format off */
LISP_MODULE_SETUP(set, <-, ANY)
/* clang-format */

// vim: tw=80:sw=2:ts=2:sts=2:et
