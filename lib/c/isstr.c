#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

static bool
lisp_is_string(const atom_t cell)
{
  TRACE_SEXP(cell);
  /*
   * Default case.
   */
  if (IS_NULL(cell)) {
    return true;
  }
  /*
   * Must be a list.
   */
  if (!IS_PAIR(cell)) {
    return false;
  }
  /*
   * Check CAR.
   */
  if (!IS_CHAR(cell->pair.car)) {
    return false;
  }
  /*
   * Recurse over CDR.
   */
  return lisp_is_string(cell->pair.cdr);
}

PREDICATE_GEN(str, lisp_is_string);
LISP_PLUGIN_REGISTER(isstr, str?)
