#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_function_set(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(cell, arguments, @);
  /*
   * Lookup the symbol and the value.
   */
  atom_t sym = lisp_eval(closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  atom_t val = lisp_eval(closure, lisp_car(cdr));
  X(cdr); X(cell);
  /*
   * Set the symbol in the current closure.
   */
  FOREACH(closure, a) {
    atom_t car = a->car;
    if (lisp_symbol_match(CAR(car), sym)) {
      X(CDR(car));
      CDR(car) = UP(val);
      X(sym);
      return val;
    }
    NEXT(a);
  }
  /*
   * If not found, set the symbol in the GLOBALS.
   */
  FOREACH(GLOBALS, b) {
    atom_t car = b->car;
    if (lisp_symbol_match(CAR(car), sym)) {
      X(CDR(car));
      CDR(car) = UP(val);
      X(sym);
      return val;
    }
    NEXT(b);
  }
  /*
   * Return NIL if not found.
   */
  X(sym); X(val);
  return UP(NIL);
}

LISP_PLUGIN_REGISTER(set, <-, @)
