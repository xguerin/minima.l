#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <string.h>

static atom_t
lisp_function_setq(const atom_t closure, const atom_t cell)
{
  atom_t sym = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  atom_t car = lisp_car(cdr);
  X(cdr);
  /*
   * Don't set anything if NIL.
   */
  if (unlikely(IS_NULL(car))) {
    X(sym);
    return car;
  }
  /*
   * Call SETQ.
   */
  atom_t res = lisp_eval(closure, car);
  GLOBALS = lisp_setq(GLOBALS, sym, UP(res));
  return res;
}

LISP_REGISTER(setq, setq)
