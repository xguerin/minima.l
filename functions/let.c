#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <string.h>

static atom_t
lisp_let_bind(const atom_t closure, const atom_t cell)
{
  TRACE_SEXP(cell);
  /*
   * Check if the cell is a pair.
   */
  if (unlikely(!IS_PAIR(cell))) {
    X(cell);
    TRACE_SEXP(closure);
    return closure;
  }
  /*
   * Grab CAR and CDR.
   */
  atom_t car = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  /*
   * Process the CAR.
   */
  if (likely(IS_PAIR(car))) {
    atom_t arg = lisp_car(car);
    atom_t val = lisp_eval(closure, lisp_cdr(car));
    X(car);
    atom_t newc = lisp_bind(closure, arg, val);
    return lisp_let_bind(newc, cdr);
  }
  /*
   */
  X(car); X(cdr);
  TRACE_SEXP(closure);
  return closure;
}

static atom_t
lisp_let(const atom_t closure, const atom_t cell)
{
  TRACE_SEXP(cell);
  /*
   * Get the bind list and the prog.
   */
  atom_t bind = lisp_car(cell);
  atom_t prog = lisp_cdr(cell);
  X(cell);
  /*
   * Recursively apply the bind list.
   */
  TRACE_SEXP(closure);
  atom_t newl = lisp_dup(closure);
  atom_t newc = lisp_let_bind(newl, bind);
  TRACE_SEXP(newc);
  /*
   * Evaluate the prog with the new bind list.
   */
  atom_t res = lisp_prog(newc, prog, UP(NIL));
  X(newc);
  return res;
}

atom_t
lisp_function_let(const atom_t closure, const atom_t cell)
{
  return lisp_let(closure, cell);
}

LISP_REGISTER(let, let)
