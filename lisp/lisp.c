#include "lisp.h"
#include <stdio.h>

static void lisp_print_cell(const cell_t cell);
static void lisp_print_list(const cell_t cell);

static void lisp_print_list(const cell_t cell)
{
  /*
   * Process CAR.
   */
  switch (cell->car.type) {
    case T_NUMBER: printf("%lld", cell->car.value.number); break;
    case T_STRING: printf("\"%s\"", cell->car.value.string); break;
    case T_SYMBOL: printf("%s", cell->car.value.symbol); break;
    case T_CELL: lisp_print_cell(cell->car.value.cell); break;
    case T_NIL : break;
  }
  /*
   * Process CDR.
   */
  switch (cell->cdr.type) {
    case T_NUMBER: printf(" . %lld", cell->cdr.value.number); break;
    case T_STRING: printf(" . \"%s\"", cell->cdr.value.string); break;
    case T_SYMBOL: printf(" . %s", cell->cdr.value.symbol); break;
    case T_CELL: printf(" "); lisp_print_list(cell->cdr.value.cell); break;
    case T_NIL : break;
  }
}

static void lisp_print_cell(const cell_t cell)
{
  printf("(");
  lisp_print_list(cell);
  printf(")");
}

void
lisp_print(const cell_t cell)
{
  lisp_print_cell(cell);
  printf("\n");
}
