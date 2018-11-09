#include "lisp.h"
#include <stdlib.h>
#include <string.h>

/*
 * Print function.
 */

static void
lisp_print_atom(FILE * const fp, const atom_t atom, const bool alter)
{
  switch (atom->type) {
    case T_NIL:
      fprintf(fp, "NIL");
      break;
    case T_TRUE:
      fprintf(fp, "T");
      break;
    case T_CHAR:
      if ((char)atom->number == '\'') {
        fprintf(fp, "'\\''");
      }
      else {
        fprintf(fp, "'%c'", (int)atom->number);
      }
      break;

    case T_PAIR:
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
      break;
    case T_NUMBER:
#ifdef __MACH__
      fprintf(fp, "%lld", atom->number);
#else
      fprintf(fp, "%ld", atom->number);
#endif
      break;
    case T_STRING:
      fprintf(fp, "\"%s\"", atom->string);
      break;
    case T_SYMBOL: {
      char bsym[17] = { 0 };
      strncpy(bsym, atom->symbol.val, 16);
      fprintf(fp, "%s", bsym);
      break;
    }
    case T_WILDCARD:
      fprintf(fp, "_");
      break;
  }
}

void
lisp_print(FILE * const fp, const atom_t atom)
{
  lisp_print_atom(fp, atom, true);
  fprintf(fp, "\n");
}
