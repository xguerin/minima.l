#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <errno.h>
#include <unistd.h>

static atom_t USED
lisp_function_dup(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, ANY);
  /*
   * Grab the arguments.
   */
  atom_t car = lisp_eval(lisp, C, lisp_car(lisp, ANY));
  atom_t cdr = lisp_cdr(lisp, ANY);
  atom_t tgt = lisp_eval(lisp, C, lisp_car(lisp, cdr));
  X(lisp, cdr);
  /*
   * Call DUP if target is NIL.
   */
  if (IS_NULL(tgt)) {
    int ret = dup((int)car->number);
    X(lisp, tgt, car);
    return lisp_make_number(lisp, ret < 0 ? errno : ret);
  }
  /*
   * Otherwise call DUP2.
   */
  int ret = dup2((int)car->number, (int)tgt->number);
  X(lisp, tgt, car);
  return lisp_make_number(lisp, ret < 0 ? errno : ret);
}

LISP_MODULE_SETUP(dup, dup, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
