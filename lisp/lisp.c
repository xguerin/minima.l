#include "lisp.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Helper functions.
 */

static void
lisp_free_entry(const uintptr_t entry)
{
  switch (GET_TYPE(entry)) {
    case T_CELL: lisp_free(GET_PNTR(cell_t, entry)); break;
    case T_STRING:
    case T_SYMBOL: free(GET_PNTR(char *, entry)); break;
    default: break;
  }
}

void
lisp_free(const cell_t cell)
{
  if (cell != NULL) {
    lisp_free_entry(cell->car);
    lisp_free_entry(cell->cdr);
    free(cell);
  }
}

static void lisp_print_cell(const cell_t cell);
static void lisp_print_list(const cell_t cell);

static void lisp_print_list(const cell_t cell)
{
  /*
   * Process CAR.
   */
  switch (GET_TYPE(cell->car)) {
    case T_NUMBER:        printf("%lld", GET_NUMB(cell->car)); break;
    case T_STRING:        printf("\"%s\"", GET_PNTR(char *, cell->car)); break;
    case T_SYMBOL:        printf("%s", GET_PNTR(char *, cell->car)); break;
    case T_SYMBOL_INLINE: printf("%s", GET_SYMB(cell->car)); break;
    case T_CELL:          lisp_print_cell(GET_PNTR(cell_t, cell->car)); break;
    case T_NIL :          break;
  }
  /*
   * Process CDR.
   */
  switch (GET_TYPE(cell->cdr)) {
    case T_NUMBER:        printf(" . %lld", GET_NUMB(cell->cdr)); break;
    case T_STRING:        printf(" . \"%s\"", GET_PNTR(char *, cell->cdr)); break;
    case T_SYMBOL:        printf(" . %s", GET_PNTR(char *, cell->cdr)); break;
    case T_SYMBOL_INLINE: printf(" . %s", GET_SYMB(cell->cdr)); break;
    case T_CELL:          printf(" "); lisp_print_list(GET_PNTR(cell_t, cell->cdr)); break;
    case T_NIL :          break;
  }
}

static void lisp_print_cell(const cell_t cell)
{
  printf("(");
  if (cell != NULL) {
    lisp_print_list(cell);
  }
  printf(")");
}

void
lisp_print(const cell_t cell)
{
  lisp_print_cell(cell);
  printf("\n");
}
