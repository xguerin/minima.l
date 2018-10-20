#include "lisp.h"

static void lisp_print_cell(FILE * const fp, const cell_t cell);
static void lisp_print_list(FILE * const fp, const cell_t cell);

static void lisp_print_list(FILE * const fp, const cell_t cell)
{
  /*
   * Process CAR.
   */
  switch (GET_TYPE(cell->car)) {
    case T_NIL : {
      printf("NIL");
      break;
    }
    case T_TRUE : {
      printf("T");
      break;
    }
    case T_NUMBER: {
      fprintf(fp, "%lld", GET_NUMB(cell->car));
      break;
    }
    case T_STRING: {
      fprintf(fp, "\"%s\"", GET_PNTR(char *, cell->car));
      break;
    }
    case T_SYMBOL: {
      fprintf(fp, "%s", GET_PNTR(char *, cell->car));
      break;
    }
    case T_SYMBOL_INLINE: {
      fprintf(fp, "%s", GET_SYMB(cell->car));
      break;
    }
    case T_LIST: {
      fputc('(', fp);
      lisp_print_cell(fp, GET_PNTR(cell_t, cell->car));
      fputc(')', fp);
      break;
    }
  }
  /*
   * Process CDR.
   */
  switch (GET_TYPE(cell->cdr)) {
    case T_NIL : {
      break;
    }
    case T_TRUE : {
      printf("T");
      break;
    }
    case T_NUMBER: {
      fprintf(fp, " . %lld", GET_NUMB(cell->cdr));
      break;
    }
    case T_STRING: {
      fprintf(fp, " . \"%s\"", GET_PNTR(char *, cell->cdr));
      break;
    }
    case T_SYMBOL: {
      fprintf(fp, " . %s", GET_PNTR(char *, cell->cdr));
      break;
    }
    case T_SYMBOL_INLINE: {
      fprintf(fp, " . %s", GET_SYMB(cell->cdr));
      break;
    }
    case T_LIST: {
      fprintf(fp, " ");
      lisp_print_list(fp, GET_PNTR(cell_t, cell->cdr));
      break;
    }
  }
}

static void lisp_print_cell(FILE * const fp, const cell_t cell)
{
  lisp_print_list(fp, cell);
}

void
lisp_print(FILE * const fp, const cell_t cell)
{
  lisp_print_cell(fp, cell);
  printf("\n");
}
