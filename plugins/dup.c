#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

atom_t
lisp_function_dup(const atom_t closure, const atom_t cell)
{
  atom_t car = lisp_eval(closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  atom_t tgt = lisp_eval(closure, lisp_car(cdr));
  X(cdr); X(cell);
  /*
   * Call DUP if target is NIL.
   */
  if (IS_NULL(tgt)) {
    int ret = dup(car->number);
    X(tgt); X(car);
    return lisp_make_number(ret < 0 ? errno : ret);
  }
  /*
   * Otherwise call DUP2.
   */
  int ret = dup2(car->number, tgt->number);
  X(tgt); X(car);
  return lisp_make_number(ret < 0 ? errno : ret);
}

LISP_REGISTER(dup, dup)
