#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/tree.h>

static atom_t USED
lisp_function_set(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, ANY);
  /*
   * Lookup the symbol and the value.
   */
  atom_t sym = lisp_eval(lisp, C, lisp_car(lisp, ANY));
  atom_t cdr = lisp_cdr(lisp, ANY);
  atom_t val = lisp_eval(lisp, C, lisp_car(lisp, cdr));
  /*
   * Check if sym is a symbol.
   */
  if (unlikely(!IS_SYMB(sym))) {
    X(lisp, sym, val);
    return lisp_make_nil(lisp);
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
      X(lisp, sym);
      return res;
    }
    NEXT(c0);
  }
  /*
   * Look second in globals.
   */
  atom_t elt = lisp_cons(lisp, sym, val);
  atom_t res = lisp_tree_upd(lisp, lisp->globals, elt);
  if (!IS_NULL(res)) {
    return res;
  }
  X(lisp, res);
  /*
   * The symbol was not found.
   */
  X(lisp, sym, val);
  return lisp_make_nil(lisp);
}

/* clang-format off */
LISP_MODULE_SETUP(set, <-, ANY)
/* clang-format */

// vim: tw=80:sw=2:ts=2:sts=2:et
