#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t
lisp_function_sym(const lisp_t lisp, const atom_t closure,
                  const atom_t arguments)
{
  LISP_LOOKUP(car, arguments, X);
  /*
   * Check that the argument is a string.
   */
  if (unlikely(!(IS_PAIR(car) && lisp_is_string(car)))) {
    X(car);
    return UP(NIL);
  }
  /*
   * Process the string.
   */
  char buffer[17];
  size_t len = lisp_make_cstring(car, buffer, LISP_SYMBOL_LENGTH, 0);
  X(car);
  if (len == 0) {
    return UP(NIL);
  }
  MAKE_SYMBOL_STATIC(symb, buffer, len);
  return lisp_make_symbol(symb);
}

LISP_MODULE_SETUP(sym, sym, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
