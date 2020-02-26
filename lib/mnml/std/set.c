#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_set(const lisp_t lisp, const atom_t closure, const atom_t sym,
         const atom_t val)
{
  /*
   * Default condition, check the global environemnt.
   */
  if (IS_NULL(closure)) {
    FOREACH(lisp->GLOBALS, g)
    {
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
  FOREACH(CAR(closure), a)
  {
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
  return lisp_set(lisp, CDR(closure), sym, val);
}

static atom_t
lisp_function_set(const lisp_t lisp, const atom_t closure,
                  const atom_t arguments)
{
  LISP_LOOKUP(cell, arguments, @);
  /*
   * Lookup the symbol and the value.
   */
  atom_t sym = lisp_eval(lisp, closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  atom_t val = lisp_eval(lisp, closure, lisp_car(cdr));
  X(cdr, cell);
  /*
   * Set the symbol in the closure stack.
   */
  return lisp_set(lisp, closure, sym, val);
}

/* clang-format off */
LISP_MODULE_SETUP(set, <-, @)
/* clang-format */

// vim: tw=80:sw=2:ts=2:sts=2:et
