#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <string.h>

atom_t
lisp_function_sym(const atom_t closure, const atom_t cell)
{
  char buffer[17];
  atom_t car = lisp_car(cell);
  size_t len = lisp_make_cstring(car, buffer, 16, 0);
  X(car); X(cell);
  if (len == 0) {
    return UP(NIL);
  }
  MAKE_SYMBOL_STATIC(symb, buffer, len);
  return lisp_make_symbol(symb);
}

LISP_REGISTER(sym, sym)
