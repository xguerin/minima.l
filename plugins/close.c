#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

atom_t
lisp_function_close(const atom_t closure, const atom_t cell)
{
  atom_t car = lisp_eval(closure, lisp_car(cell));
  int ret = close(car->number);
  X(car); X(cell);
  return lisp_make_number(ret < 0 ? errno : ret);
}

LISP_REGISTER(close, close)
