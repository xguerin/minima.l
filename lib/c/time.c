#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t
lisp_function_time(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(car, arguments, X);
  uint64_t begin = lisp_timestamp();
  atom_t res = lisp_eval(closure, car);
  uint64_t end = lisp_timestamp();
  X(res);
  return lisp_make_number(end - begin);
}

LISP_PLUGIN_REGISTER(time, time, X)
