#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

static atom_t
lisp_function_setq(const atom_t closure, const atom_t cell)
{
  atom_t sym = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  atom_t res = lisp_eval(closure, lisp_car(cdr));
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
  GLOBALS = lisp_setq(GLOBALS, lisp_cons(sym, res));
  X(sym);
  return res;
}

LISP_REGISTER(setq, setq)
