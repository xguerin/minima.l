#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/utils.h>
#include <stdlib.h>
#include <string.h>

#ifdef LISP_ENABLE_DEBUG

/*
 * Debug flags.
 */

bool MNML_DEBUG = false;
bool MNML_DEBUG_BIND = false;
bool MNML_DEBUG_CHAN = false;
bool MNML_DEBUG_CLOS = false;
bool MNML_DEBUG_CONS = false;
bool MNML_DEBUG_EVAL = false;
bool MNML_DEBUG_MAKE = false;
bool MNML_DEBUG_MODL = false;
bool MNML_DEBUG_REFC = false;
bool MNML_DEBUG_SLOT = false;
bool MNML_DEBUG_SLAB = false;
bool MNML_DEBUG_TAIL = false;

static void
lisp_debug_set_flag(const char* const flag)
{
  MNML_DEBUG_BIND = MNML_DEBUG_BIND || strcmp(flag, "BIND") == 0;
  MNML_DEBUG_CHAN = MNML_DEBUG_CHAN || strcmp(flag, "CHAN") == 0;
  MNML_DEBUG_CLOS = MNML_DEBUG_CLOS || strcmp(flag, "CLOS") == 0;
  MNML_DEBUG_CONS = MNML_DEBUG_CONS || strcmp(flag, "CONS") == 0;
  MNML_DEBUG_EVAL = MNML_DEBUG_EVAL || strcmp(flag, "EVAL") == 0;
  MNML_DEBUG_MAKE = MNML_DEBUG_MAKE || strcmp(flag, "MAKE") == 0;
  MNML_DEBUG_MODL = MNML_DEBUG_MODL || strcmp(flag, "MODL") == 0;
  MNML_DEBUG_REFC = MNML_DEBUG_REFC || strcmp(flag, "REFC") == 0;
  MNML_DEBUG_SLAB = MNML_DEBUG_SLAB || strcmp(flag, "SLAB") == 0;
  MNML_DEBUG_SLOT = MNML_DEBUG_SLOT || strcmp(flag, "SLOT") == 0;
  MNML_DEBUG_TAIL = MNML_DEBUG_SLOT || strcmp(flag, "TAIL") == 0;
}

void
lisp_debug_parse_flags()
{
  const char* DEBUG = getenv("MNML_DEBUG");
  if (DEBUG != NULL) {
    MNML_DEBUG = true;
    FOR_EACH_TOKEN(DEBUG, ",", flag, lisp_debug_set_flag(flag));
  }
}

/*
 * Debug function.
 */

static void
lisp_debug_atom(FILE* const fp, const atom_t atom, const bool alter,
                const size_t level)
{
  /*
   * Process the current atom.
   */
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
        size_t len = lisp_make_cstring(atom, buffer, 1024, 0);
        fprintf(fp, "\"%.*s\"", (int)len, buffer);
        break;
      }
      /*
       * Return an ellipsis if level == 0.
       */
      if (level == 0) {
        fprintf(fp, "…");
        break;
      }
      /*
       * Process the list.
       */
      if (alter) {
        fprintf(fp, "(");
      }
      /*
       * Print CAR.
       */
      if (IS_WEAKREF(atom)) {
        fprintf(fp, "@");
      } else {
        lisp_debug_atom(fp, CAR(atom), true, level - 1);
      }
      /*
       * Print CDR.
       */
      if (!IS_NULL(CDR(atom))) {
        if (IS_PAIR(CDR(atom))) {
          fprintf(fp, " ");
        } else {
          fprintf(fp, " . ");
        }
        lisp_debug_atom(fp, CDR(atom), false, level);
      }
      /*
       */
      if (alter) {
        fprintf(fp, ")");
      }
      break;
    case T_NUMBER:
#if defined(__MACH__) || defined(__OpenBSD__)
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
    default:
      TRACE("Unknown-type error");
      abort();
  }
}

void
lisp_debug(FILE* const fp, const atom_t atom, const size_t level)
{
  if (MNML_DEBUG) {
    lisp_debug_atom(fp, atom, true, level);
    fprintf(fp, "\n");
  }
}

#endif

// vim: tw=80:sw=2:ts=2:sts=2:et
