#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <string.h>
#include <unistd.h>

static atom_t
lisp_prinl_all(const atom_t closure, const atom_t cell, const atom_t result)
{
  /*
   */
  atom_t car = lisp_eval(closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  lisp_prin(closure, car);
  X(cell); X(result);
  /*
   */
  if (IS_NULL(cdr)) {
    X(cdr);
    return car;
  }
  /*
   */
  return lisp_prinl_all(closure, cdr, car);
}

atom_t
lisp_function_prinl(const atom_t closure, const atom_t cell)
{
  atom_t res = lisp_prinl_all(closure, cell, UP(NIL));
  write(CAR(CAR(ICHAN))->number, "\n", 1);
  return res;
}

LISP_REGISTER(prinl, prinl)
