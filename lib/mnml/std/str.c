#include "mnml/maker.h"
#include "mnml/types.h"
#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t
to_string(const lisp_t lisp, const atom_t cell)
{
  char buffer[17] = { 0 };
  strncpy(buffer, cell->symbol.val, LISP_SYMBOL_LENGTH);
  return lisp_make_string(lisp, buffer, strlen(buffer));
}

static atom_t USED
lisp_function_str(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, X);
  /*
   * Check if the argument is valid.
   */
  if (!IS_SCOP(X) && !IS_SYMB(X)) {
    return lisp_make_nil(lisp);
  }
  /*
   * Convert the scoped symbol.
   */
  if (IS_SCOP(X)) {
    /*
     * Get the parts.
     */
    atom_t nsp = lisp_scope_get_name(lisp, X);
    atom_t sym = lisp_scope_get_symb(lisp, X);
    /*
     * Convert the namespace.
     */
    atom_t st0 = to_string(lisp, nsp);
    atom_t st1 = to_string(lisp, sym);
    X(lisp->slab, nsp, sym);
    /*
     * Return the result.
     */
    return lisp_cons(lisp, st0, st1);
  }
  /*
   * Convert the symbol.
   */
  char buffer[17] = { 0 };
  strncpy(buffer, X->symbol.val, LISP_SYMBOL_LENGTH);
  return lisp_make_string(lisp, buffer, strlen(buffer));
}

LISP_MODULE_SETUP(str, str, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
