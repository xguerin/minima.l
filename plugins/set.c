#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

static atom_t
lisp_function_set(const atom_t closure, const atom_t cell)
{
  atom_t sym = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  atom_t val = lisp_car(cdr);
  X(cdr); X(cell);
  /*
   * Set the symbol in the current closure.
   */
  FOREACH(closure, a) {
    atom_t car = a->car;
    if (lisp_symbol_match(CAR(car), sym)) {
      X(car);
      a->car = lisp_cons(sym, val);
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
      X(car);
      b->car = lisp_cons(sym, val);
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

LISP_REGISTER(set, <-)
