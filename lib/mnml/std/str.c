#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t USED
lisp_function_str(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, car, closure, X);
  char buffer[17] = { 0 };
  strncpy(buffer, car->symbol.val, LISP_SYMBOL_LENGTH);
  X(car);
  return lisp_make_string(buffer, strlen(buffer));
}

LISP_MODULE_SETUP(str, str, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
