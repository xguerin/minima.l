#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <unistd.h>

static bool
lisp_close(const lisp_t lisp, const atom_t closure, const atom_t cell,
           const bool res)
{
  if (!IS_NULL(cell)) {
    atom_t car = lisp_eval(lisp, closure, lisp_car(lisp, cell));
    atom_t cdr = lisp_cdr(lisp, cell);
    int s = close((int)car->number);
    X(lisp, car, cell);
    return lisp_close(lisp, closure, cdr, res && s == 0);
  }
  /*
   */
  X(lisp, cell);
  return res;
}

static atom_t USED
lisp_function_close(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, ANY);
  bool res = lisp_close(lisp, C, UP(ANY), true);
  return res ? lisp_make_true(lisp) : lisp_make_nil(lisp);
}

LISP_MODULE_SETUP(close, close, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
