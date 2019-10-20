#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t
lisp_function_sym(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(car, arguments, X);
  char buffer[17];
  size_t len = lisp_make_cstring(car, buffer, LISP_SYMBOL_LENGTH, 0);
  X(car);
  if (len == 0) {
    return UP(NIL);
  }
  MAKE_SYMBOL_STATIC(symb, buffer, len);
  return lisp_make_symbol(symb);
}

LISP_PLUGIN_REGISTER(sym, sym, X)
