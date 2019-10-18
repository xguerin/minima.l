#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <stdlib.h>
#include <string.h>

#ifdef LISP_ENABLE_DEBUG

bool MNML_DEBUG        = false;
bool MNML_VERBOSE_CONS = false;
bool MNML_VERBOSE_RC   = false;
bool MNML_VERBOSE_SLOT = false;
bool MNML_VERBOSE_SLAB = false;

/*
 * Debug function.
 */

static void
lisp_debug_atom(FILE * const fp, const atom_t atom, const bool alter)
{
  switch (atom->type) {
    case T_NIL:
      fprintf(fp, "NIL");
      break;
    case T_TRUE:
      fprintf(fp, "T");
      break;
    case T_CHAR: {
      const char c = (char)atom->number;
      switch (c) {
        case '\n':
          fprintf(fp, ";\\n");
          break;
        default:
          fprintf(fp, "^%c", c);
      }
      break;
    }
    case T_PAIR:
      if (alter) fprintf(fp, "(");
      /*
       * Print CAR.
       */
      lisp_debug_atom(fp, CAR(atom), true);
      /*
       * Print CDR.
       */
      if (CDR(atom) != NIL) {
        if (IS_PAIR(CDR(atom))) {
          fprintf(fp, " ");
        }
        else {
          fprintf(fp, " . ");
        }
        lisp_debug_atom(fp, CDR(atom), false);
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
    case T_SYMBOL: {
      char bsym[17] = { 0 };
      strncpy(bsym, atom->symbol.val, LISP_SYMBOL_LENGTH);
      fprintf(fp, "%s", bsym);
      break;
    }
    case T_WILDCARD:
      fprintf(fp, "_");
      break;
  }
}

void
lisp_debug(FILE * const fp, const atom_t atom)
{
  if (MNML_DEBUG) {
    lisp_debug_atom(fp, atom, true);
    fprintf(fp, "\n");
  }
}

#endif
