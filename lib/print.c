#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <unistd.h>

static atom_t
lisp_print_all(const lisp_t lisp, const atom_t closure, const atom_t cell,
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
  lisp_prin(closure, car, true);
  if (!IS_NULL(cdr)) {
    fwrite(" ", 1, 1, (FILE*)CAR(CAR(OCHAN))->number);
  }
  X(cell, result);
  return lisp_print_all(lisp, closure, cdr, car);
}

static atom_t
lisp_function_print(const lisp_t lisp, const atom_t closure,
                    const atom_t arguments)
{
  LISP_LOOKUP(cell, arguments, @);
  return lisp_print_all(lisp, closure, cell, UP(NIL));
}

LISP_PLUGIN_REGISTER(print, print, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
