#include "lisp.h"
#include "utils.h"
#include <limits.h>

void
lisp_make_string(const atom_t cell, char * const buffer, const size_t idx)
{
  /*
   * Terminate the string.
   */
  if (IS_NULL(cell) || idx == PATH_MAX) {
    *buffer = '\0';
    return;
  }
  /*
   * Process the chars.
   */
  lisp_make_string(CDR(cell), buffer + 1, idx + 1);
  *buffer = CAR(cell)->number;
}
