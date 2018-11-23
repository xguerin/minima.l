#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
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
    write(CAR(CAR(OCHAN))->number, " ", 1);
  }
  X(cell); X(result);
  return lisp_printl_all(closure, cdr, car);
}

atom_t
lisp_function_printl(const atom_t closure, const atom_t cell)
{
  atom_t res = lisp_printl_all(closure, cell, UP(NIL));
  write(CAR(CAR(OCHAN))->number, "\n", 1);
  return res;
}

LISP_PLUGIN_REGISTER(printl, printl)
