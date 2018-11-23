#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <string.h>

atom_t
lisp_function_time(const atom_t closure, const atom_t cell)
{
  atom_t car = lisp_car(cell);
  uint64_t begin = lisp_timestamp();
  atom_t res = lisp_eval(closure, car);
  uint64_t end = lisp_timestamp();
  X(res); X(cell);
  return lisp_make_number(end - begin);
}

LISP_PLUGIN_REGISTER(time, time)
