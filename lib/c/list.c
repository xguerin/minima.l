#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

static atom_t
lisp_list(const atom_t closure, const atom_t cell)
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
    /*
     * Recursively get the result.
     */
    atom_t res = lisp_list(closure, cdr);
    atom_t evl = lisp_eval(closure, car);
    atom_t con = lisp_cons(evl, res);
    X(evl); X(res);
    return con;
  }
  /*
   */
  return cell;
}

static atom_t
lisp_function_list(const atom_t closure, const atom_t cell)
{
  return lisp_list(closure, cell);
}

LISP_PLUGIN_REGISTER(list, list)
