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
  atom_t sym = lisp_car(ANY);
  atom_t cdr = lisp_cdr(ANY);
  atom_t res = lisp_eval(lisp, C, lisp_car(cdr));
  X(cdr);
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
  atom_t tmp = GLOBALS;
  GLOBALS = lisp_setq(GLOBALS, lisp_cons(sym, res));
  X(sym, tmp);
  return res;
}

LISP_MODULE_SETUP(setq, setq, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
