#include "utils.h"
#include <lisp/lisp.h>
#include <lisp/slab.h>
#include <string.h>

static atom_t
lisp_pipe(const atom_t closure, const atom_t cell, const atom_t result)
{
  TRACE_SEXP(cell);
  /*
   */
  if (likely(IS_PAIR(cell))) {
    /*
     * Get CAR/CDR.
     */
    atom_t car = lisp_car(cell);
    atom_t cdr = lisp_cdr(cell);
    X(cell);
    atom_t lsp = lisp_cons(result, NIL);
    X(result);
    atom_t env = lisp_cons(car, lsp);
    X(car); X(lsp);
    /*
     */
    atom_t res = lisp_eval(closure, env);
    return lisp_pipe(closure, cdr, res);
  }
  /*
   */
  X(cell);
  return result;
}

static atom_t
lisp_function_pipe(const atom_t closure, const atom_t cell)
{
  return lisp_pipe(closure, cell, UP(NIL));
}

LISP_REGISTER(pipe, |>)
