#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_set(const atom_t closure, const atom_t sym, const atom_t val)
{
  TRACE_SEXP(closure);
  /*
   * Default condition, check the global environemnt.
   */
  if (IS_NULL(closure)) {
    FOREACH(GLOBALS, g) {
      atom_t car = g->car;
      if (lisp_symbol_match(CAR(car), sym)) {
        X(CDR(car));
        CDR(car) = UP(val);
        X(sym);
        return val;
      }
      NEXT(g);
    }
    /*
     * Nothing found.
     */
    X(sym, val);
    return UP(NIL);
  }
  /*
   * Look for the symbol up the closure stack.
   */
  FOREACH(CAR(closure), a) {
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
   * Check the next level.
   */
  return lisp_set(CDR(closure), sym, val);
}

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
  X(cdr, cell);
  /*
   * Set the symbol in the closure stack.
   */
  return lisp_set(closure, sym, val);
}

LISP_PLUGIN_REGISTER(set, <-, @)
