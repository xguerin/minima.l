#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/utils.h>
#include <stdlib.h>
#include <string.h>

#ifdef LISP_ENABLE_DEBUG

/*
 * Debug flags.
 */

bool MNML_DEBUG        = false;
bool MNML_VERBOSE_BIND = false;
bool MNML_VERBOSE_CHAN = false;
bool MNML_VERBOSE_CONS = false;
bool MNML_VERBOSE_MAKE = false;
bool MNML_VERBOSE_PLUG = false;
bool MNML_VERBOSE_REFC = false;
bool MNML_VERBOSE_SLOT = false;
bool MNML_VERBOSE_SLAB = false;

static void
lisp_debug_set_flag(const char * const flag)
{
  MNML_VERBOSE_BIND = MNML_VERBOSE_BIND || strcmp(flag, "BIND") == 0;
  MNML_VERBOSE_CHAN = MNML_VERBOSE_CHAN || strcmp(flag, "CHAN") == 0;
  MNML_VERBOSE_CONS = MNML_VERBOSE_CONS || strcmp(flag, "CONS") == 0;
  MNML_VERBOSE_MAKE = MNML_VERBOSE_MAKE || strcmp(flag, "MAKE") == 0;
  MNML_VERBOSE_PLUG = MNML_VERBOSE_PLUG || strcmp(flag, "PLUG") == 0;
  MNML_VERBOSE_REFC = MNML_VERBOSE_REFC || strcmp(flag, "REFC") == 0;
  MNML_VERBOSE_SLAB = MNML_VERBOSE_SLAB || strcmp(flag, "SLAB") == 0;
  MNML_VERBOSE_SLOT = MNML_VERBOSE_SLOT || strcmp(flag, "SLOT") == 0;
}

void
lisp_debug_parse_flags()
{
  const char * DEBUG = getenv("MNML_DEBUG");
  if (DEBUG != NULL) {
    MNML_DEBUG = true;
    FOR_EACH_TOKEN(DEBUG, ",", flag, lisp_debug_set_flag(flag));
  }
}


/*
 * Debug function.
 */

static void
lisp_debug_atom(FILE * const fp, const atom_t atom, bool alter)
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
        case '\033':
          fprintf(fp, "^\\e");
          break;
        case '\n':
          fprintf(fp, "^\\n");
          break;
        case '\r':
          fprintf(fp, "^\\r");
          break;
        case '\t':
          fprintf(fp, "^\\t");
          break;
        default:
          fprintf(fp, "^%c", c);
      }
      break;
    }
    case T_PAIR:
      /*
       * Check if it's a string.
       */
      if (!IS_NULL(atom) && lisp_is_string(atom)) {
        char buffer[1024];
        int len = lisp_make_cstring(atom, buffer, 1024, 0);
        fprintf(fp, "\"%.*s\"", len, buffer);
        break;
      }
      /*
       * Process the list.
       */
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
