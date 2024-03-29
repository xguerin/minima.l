#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_setq(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, ANY);
  /*
   * Extract the symbol and the value.
   */
  atom_t sym = lisp_car(lisp, ANY);
  atom_t cdr = lisp_cdr(lisp, ANY);
  atom_t res = lisp_eval(lisp, C, lisp_car(lisp, cdr));
  X(lisp, cdr);
  /*
   * Don't set anything if NIL.
   */
  if (unlikely(IS_NULL(res))) {
    X(lisp, sym);
    return res;
  }
  /*
   * Call SETQ.
   */
  atom_t elt = lisp_cons(lisp, sym, UP(res));
  lisp->globals = lisp_setq(lisp, lisp->globals, elt);
  return res;
}

LISP_MODULE_SETUP(setq, setq, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
