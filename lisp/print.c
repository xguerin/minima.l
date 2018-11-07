#include "lisp.h"

typedef void (* lisp_printer_t)(FILE * const fp, const atom_t atom,
                                const bool alter);

static void
lisp_print_atom(FILE * const fp, const atom_t atom, const bool alter);

/*
 * PRINT jump tables.
 */

static void
lisp_print_nil(FILE * const fp, const atom_t atom, const bool alter)
{
  fprintf(fp, "NIL");
}

static void
lisp_print_true(FILE * const fp, const atom_t atom, const bool alter)
{
  fprintf(fp, "T");
}

static void
lisp_print_star(FILE * const fp, const atom_t atom, const bool alter)
{
  fprintf(fp, "_");
}

static void
lisp_print_number(FILE * const fp, const atom_t atom, const bool alter)
{
#ifdef __MACH__
  fprintf(fp, "%lld", atom->number);
#else
  fprintf(fp, "%ld", atom->number);
#endif
}

static void
lisp_print_string(FILE * const fp, const atom_t atom, const bool alter)
{
  fprintf(fp, "\"%s\"", atom->string);
}

static void
lisp_print_symbol(FILE * const fp, const atom_t atom, const bool alter)
{
  fprintf(fp, "%s", atom->string);
}

static void
lisp_print_pair(FILE * const fp, const atom_t atom, const bool alter)
{
  if (alter) fprintf(fp, "(");
  /*
   * Print CAR.
   */
  lisp_print_atom(fp, atom->pair.car, true);
  /*
   * Print CDR.
   */
  if (atom->pair.cdr->type != T_NIL) {
    if (atom->pair.cdr->type == T_PAIR) {
      fprintf(fp, " ");
    }
    else {
      fprintf(fp, " . ");
    }
    lisp_print_atom(fp, atom->pair.cdr, false);
  }
  /*
   */
  if (alter) fprintf(fp, ")");
}

static lisp_printer_t lisp_print_table[8] =
{
  [T_NIL     ] = lisp_print_nil,
  [T_TRUE    ] = lisp_print_true,
  [T_PAIR    ] = lisp_print_pair,
  [T_NUMBER  ] = lisp_print_number,
  [T_STRING  ] = lisp_print_string,
  [T_SYMBOL  ] = lisp_print_symbol,
  [T_WILDCARD] = lisp_print_star,
};

/*
 * Print function.
 */

static void
lisp_print_atom(FILE * const fp, const atom_t atom, const bool alter)
{
  lisp_print_table[atom->type](fp, atom, alter);
}

void
lisp_print(FILE * const fp, const atom_t atom)
{
  lisp_print_atom(fp, atom, true);
  printf("\n");
}
