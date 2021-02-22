#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t USED
lisp_function_time(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, ANY);
  uint64_t result;
  uint64_t begin = lisp_timestamp();
  if (IS_NULL(ANY)) {
    result = begin;
  } else {
    atom_t car = lisp_car(lisp, ANY);
    atom_t res = lisp_eval(lisp, C, car);
    result = lisp_timestamp() - begin;
    X(lisp->slab, res);
  }
  return lisp_make_number(lisp, result);
}

LISP_MODULE_SETUP(time, time, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
