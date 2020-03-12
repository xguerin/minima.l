#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t
lisp_function_time(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, arg, closure, @);
  uint64_t result;
  uint64_t begin = lisp_timestamp();
  if (IS_NULL(arg)) {
    result = begin;
    X(arg);
  } else {
    atom_t car = lisp_car(arg);
    atom_t res = lisp_eval(lisp, closure, car);
    result = lisp_timestamp() - begin;
    X(arg, res);
  }
  return lisp_make_number(result);
}

LISP_MODULE_SETUP(time, time, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
