#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <errno.h>
#include <unistd.h>

static atom_t
lisp_function_dup(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(cell, closure, @);
  /*
   * Grab the arguments.
   */
  atom_t car = lisp_eval(lisp, closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  atom_t tgt = lisp_eval(lisp, closure, lisp_car(cdr));
  X(cdr, cell);
  /*
   * Call DUP if target is NIL.
   */
  if (IS_NULL(tgt)) {
    int ret = dup(car->number);
    X(tgt, car);
    return lisp_make_number(ret < 0 ? errno : ret);
  }
  /*
   * Otherwise call DUP2.
   */
  int ret = dup2(car->number, tgt->number);
  X(tgt, car);
  return lisp_make_number(ret < 0 ? errno : ret);
}

LISP_MODULE_SETUP(dup, dup, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
