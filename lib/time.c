#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t
lisp_function_time(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(arg, arguments, @);
  uint64_t result;
  uint64_t begin = lisp_timestamp();
  if (IS_NULL(arg)) {
    result = begin;
    X(arg);
  } else {
    atom_t car = lisp_car(arg);
    atom_t res = lisp_eval(closure, car);
    result = lisp_timestamp() - begin;
    X(arg); X(car); X(res);
  }
  return lisp_make_number(result);
}

LISP_PLUGIN_REGISTER(time, time, @)
