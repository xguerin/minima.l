#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_function_setq(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, cell, closure, @);
  /*
   * Extract the symbol and the value.
   */
  atom_t sym = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  atom_t res = lisp_eval(lisp, closure, lisp_car(cdr));
  X(cell, cdr);
  /*
   * Don't set anything if NIL.
   */
  if (unlikely(IS_NULL(res))) {
    X(sym);
    return res;
  }
  /*
   * Call SETQ.
   */
  atom_t tmp = lisp->GLOBALS;
  lisp->GLOBALS = lisp_setq(lisp->GLOBALS, lisp_cons(sym, res));
  X(sym, tmp);
  return res;
}

LISP_MODULE_SETUP(setq, setq, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
