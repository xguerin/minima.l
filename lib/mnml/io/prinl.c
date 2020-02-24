#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <stdio.h>
#include <unistd.h>

static atom_t
lisp_prinl_all(const lisp_t lisp, const atom_t closure, const atom_t cell,
               const atom_t result)
{
  if (unlikely(IS_NULL(cell))) {
    X(cell);
    return result;
  }
  /*
   */
  atom_t car = lisp_eval(lisp, closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  lisp_prin(closure, car, false);
  X(cell, result);
  return lisp_prinl_all(lisp, closure, cdr, car);
}

static atom_t
lisp_function_prinl(const lisp_t lisp, const atom_t closure,
                    const atom_t arguments)
{
  LISP_LOOKUP(cell, arguments, @);
  atom_t res = lisp_prinl_all(lisp, closure, cell, UP(NIL));
  fwrite("\n", 1, 1, (FILE*)CAR(CAR(OCHAN))->number);
  return res;
}

LISP_MODULE_SETUP(prinl, prinl, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
