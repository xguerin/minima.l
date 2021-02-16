#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_list(const lisp_t lisp, const atom_t closure, const atom_t cell)
{
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
    atom_t res = lisp_list(lisp, closure, cdr);
    atom_t evl = lisp_eval(lisp, closure, car);
    atom_t con = lisp_cons(evl, res);
    X(evl, res);
    return con;
  }
  /*
   */
  return cell;
}

static atom_t USED
lisp_function_list(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, ANY);
  return lisp_list(lisp, C, UP(ANY));
}

LISP_MODULE_SETUP(list, list, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
