#include <lisp/lisp.h>

cell_t
lisp_add(const cell_t cell)
{
  /*
   * Check overall structure.
   */
  if (cell == NULL || lisp_len(cell) != 2 || GET_TYPE(cell->cdr) != T_LIST) {
    return NULL;
  }
  /*
   * Get and check the arguments.
   */
  cell_t cdr = GET_PNTR(cell_t, cell->cdr);
  if (GET_TYPE(cell->car) != T_NUMBER || GET_TYPE(cdr->car) != T_NUMBER) {
    return NULL;
  }
  return NULL;
}
