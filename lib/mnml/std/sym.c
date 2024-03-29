#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t USED
lisp_function_sym(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, X);
  /*
   * Check that the argument is a string.
   */
  if (unlikely(!(IS_PAIR(X) && lisp_is_string(X)))) {
    return lisp_make_nil(lisp);
  }
  /*
   * Process the string.
   */
  char buffer[17];
  size_t len = lisp_make_cstring(X, buffer, LISP_SYMBOL_LENGTH, 0);
  if (len == 0) {
    return lisp_make_nil(lisp);
  }
  MAKE_SYMBOL_STATIC(symb, buffer);
  return lisp_make_symbol(lisp, symb);
}

LISP_MODULE_SETUP(sym, sym, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
