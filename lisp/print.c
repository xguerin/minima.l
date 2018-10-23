#include "lisp.h"

static void lisp_print_list(FILE * const fp, const cell_t cell);

/*
 * CAR jump tables.
 */

static void lisp_print_car_nil(FILE * const fp, const cell_t cell)
{
  fprintf(fp, "NIL");
}

static void lisp_print_car_true(FILE * const fp, const cell_t cell)
{
  fprintf(fp, "T");
}

static void lisp_print_car_star(FILE * const fp, const cell_t cell)
{
  fprintf(fp, "_");
}

static void lisp_print_car_number(FILE * const fp, const cell_t cell)
{
  fprintf(fp, "%lld", GET_NUMB(cell->car));
}

static void lisp_print_car_string(FILE * const fp, const cell_t cell)
{
  fprintf(fp, "\"%s\"", GET_PNTR(char *, cell->car));
}

static void lisp_print_car_symbol(FILE * const fp, const cell_t cell)
{
  fprintf(fp, "%s", GET_SYMB(cell->car));
}

static void lisp_print_car_list(FILE * const fp, const cell_t cell)
{
  fputc('(', fp);
  lisp_print_list(fp, GET_PNTR(cell_t, cell->car));
  fputc(')', fp);
}

static void (* lisp_print_car_table[8])(FILE * const fp, const cell_t cell) =
{
  [T_NIL          ] = lisp_print_car_nil,
  [T_LIST         ] = lisp_print_car_list,
  [T_NUMBER       ] = lisp_print_car_number,
  [T_STRING       ] = lisp_print_car_string,
  [T_SYMBOL       ] = lisp_print_car_string,
  [T_SYMBOL_INLINE] = lisp_print_car_symbol,
  [T_TRUE         ] = lisp_print_car_true,
  [T_WILDCARD     ] = lisp_print_car_star,
};

/*
 * CDR jump tables.
 */

static void lisp_print_cdr_nil(FILE * const fp, const cell_t cell)
{

}

static void lisp_print_cdr_true(FILE * const fp, const cell_t cell)
{
  fprintf(fp, " . T");
}

static void lisp_print_cdr_star(FILE * const fp, const cell_t cell)
{
  fprintf(fp, " . _");
}

static void lisp_print_cdr_number(FILE * const fp, const cell_t cell)
{
  fprintf(fp, " . %lld", GET_NUMB(cell->cdr));
}

static void lisp_print_cdr_string(FILE * const fp, const cell_t cell)
{
  fprintf(fp, " . \"%s\"", GET_PNTR(char *, cell->cdr));
}

static void lisp_print_cdr_symbol(FILE * const fp, const cell_t cell)
{
  fprintf(fp, " . %s", GET_SYMB(cell->cdr));
}

static void lisp_print_cdr_list(FILE * const fp, const cell_t cell)
{
  fprintf(fp, " ");
  lisp_print_list(fp, GET_PNTR(cell_t, cell->cdr));
}

static void (* lisp_print_cdr_table[8])(FILE * const fp, const cell_t cell) =
{
  [T_NIL          ] = lisp_print_cdr_nil,
  [T_LIST         ] = lisp_print_cdr_list,
  [T_NUMBER       ] = lisp_print_cdr_number,
  [T_STRING       ] = lisp_print_cdr_string,
  [T_SYMBOL       ] = lisp_print_cdr_string,
  [T_SYMBOL_INLINE] = lisp_print_cdr_symbol,
  [T_TRUE         ] = lisp_print_cdr_true,
  [T_WILDCARD     ] = lisp_print_cdr_star,
};

/*
 * Print function.
 */

static void lisp_print_list(FILE * const fp, const cell_t cell)
{
  lisp_print_car_table[GET_TYPE(cell->car)](fp, cell);
  lisp_print_cdr_table[GET_TYPE(cell->cdr)](fp, cell);
}

void
lisp_print(FILE * const fp, const cell_t cell)
{
  lisp_print_list(fp, cell);
  printf("\n");
}
