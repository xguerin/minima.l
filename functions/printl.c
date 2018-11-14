#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <string.h>
#include <unistd.h>

static atom_t
lisp_printl_all(const atom_t closure, const atom_t cell, const atom_t result)
{
  if (unlikely(IS_NULL(cell))) {
    X(cell);
    return result;
  }
  /*
   */
  atom_t car = lisp_eval(closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  lisp_prin(closure, car, true);
  if (!IS_NULL(cdr)) {
    write(CAR(CAR(ICHAN))->number, " ", 1);
  }
  X(cell); X(result);
  return lisp_printl_all(closure, cdr, car);
}

atom_t
lisp_function_printl(const atom_t closure, const atom_t cell)
{
  atom_t res = lisp_printl_all(closure, cell, UP(NIL));
  write(CAR(CAR(ICHAN))->number, "\n", 1);
  return res;
}

LISP_REGISTER(printl, printl)
