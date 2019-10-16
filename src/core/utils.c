#include <mnml/debug.h>
#include <mnml/maker.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <limits.h>
#include <time.h>

/*
 * Syntax error handler.
 */

error_handler_t lisp_parse_error_handler = NULL;
error_handler_t lisp_syntax_error_handler = NULL;

void parse_error()
{
  if (lisp_parse_error_handler != NULL) {
    lisp_parse_error_handler();
  }
}

void syntax_error()
{
  if (lisp_syntax_error_handler != NULL) {
    lisp_syntax_error_handler();
  }
}

/*
 * Interpreter life cycle.
 */

void
lisp_set_parse_error_handler(const error_handler_t h)
{
  lisp_parse_error_handler = h;
}

void
lisp_set_syntax_error_handler(const error_handler_t h)
{
  lisp_syntax_error_handler = h;
}

#ifdef LISP_ENABLE_DEBUG
void lisp_set_debug_flag(const char * const flag)
{
  MNML_VERBOSE_CONS = MNML_VERBOSE_CONS || strcmp(flag, "CONS") == 0;
  MNML_VERBOSE_RC   = MNML_VERBOSE_RC   || strcmp(flag, "RC")   == 0;
  MNML_VERBOSE_SLAB = MNML_VERBOSE_SLAB || strcmp(flag, "SLAB") == 0;
  MNML_VERBOSE_SLOT = MNML_VERBOSE_SLOT || strcmp(flag, "SLOT") == 0;
}
#endif

void
lisp_init()
{
  lisp_slab_allocate();
  /*
   * Create the constants.
   */
  lisp_make_nil();
  lisp_make_true();
  lisp_make_quote();
  lisp_make_wildcard();
  /*
   * Create the GLOBALS.
   */
  GLOBALS = UP(NIL);
  PLUGINS = UP(NIL);
  ICHAN   = UP(NIL);
  OCHAN   = UP(NIL);
  /*
   * Setup the debug variables.
   */
#ifdef LISP_ENABLE_DEBUG
  const char * debug = getenv("MNML_DEBUG");
  if (debug != NULL) {
    /*
     * Enable debugging.
     */
    MNML_DEBUG = true;
    /*
     * Scan the debug options.
     */
    char * copy = strdup(debug);
    char * haystack = copy;
    char * value = NULL;
    while ((value = strstr(haystack, ","))) {
      *value = 0;
      printf("### %s\n", haystack);
      lisp_set_debug_flag(haystack);
      haystack = value + 1;
    }
    printf("### %s\n", haystack);
    lisp_set_debug_flag(haystack);
    free(copy);
  }
#endif
}

void
lisp_fini()
{
  lisp_plugin_cleanup();
  X(OCHAN); X(ICHAN); X(PLUGINS); X(GLOBALS);
  X(WILDCARD); X(QUOTE); X(TRUE); X(NIL);
  TRACE("D %ld", slab.n_alloc - slab.n_free);
  LISP_COLLECT();
  lisp_slab_destroy();
}

/*
 * Return the length of a list.
 */

size_t
lisp_len(const atom_t cell)
{
  /*
   * Base case.
   */
  if (IS_NULL(cell)) {
    return 0;
  }
  /*
   * Recursive call.
   */
  atom_t cdr = lisp_cdr(cell);
  size_t len = lisp_len(cdr);
  X(cdr);
  return 1 + len;
}

/*
 * Destructively append element ELT to list LST.
 */

atom_t
lisp_append(const atom_t lst, const atom_t elt)
{
  atom_t con = lisp_cons(elt, NIL);
  atom_t res = lisp_conc(lst, con);
  X(con); X(elt); X(lst);
  return res;
}

/*
 * Shallow duplicate: 1(1X 1X ...) -> 1(2X 2X ...).
 */

atom_t
lisp_dup(const atom_t closure)
{
  if (IS_PAIR(closure)) {
    atom_t rem = lisp_dup(CDR(closure));
    atom_t res = lisp_cons(CAR(closure), rem);
    X(rem);
    return res;
  }
  return UP(closure);
}

/*
 * Merge define-site and call-site closures. The call-site closure is consumed.
 */

atom_t
lisp_merge(const atom_t defn, const atom_t call)
{
  if (IS_NULL(defn)) {
    X(defn);
    return call;
  }
  /*
   */
  atom_t elt = lisp_car(defn);
  atom_t nxt = lisp_cdr(defn);
  X(defn);
  atom_t res = lisp_setq(call, elt);
  /*
   */
  return lisp_merge(nxt, res);
}

/*
 * Make a C string from a list of characters.
 */

size_t
lisp_make_cstring(const atom_t cell, char * const buffer,
                  const size_t len, const size_t idx)
{
  /*
   * Terminate the string.
   */
  if (IS_NULL(cell) || idx == len) {
    *buffer = '\0';
    return idx;
  }
  /*
   * Process the chars.
   */
  size_t res = lisp_make_cstring(CDR(cell), buffer + 1, len, idx + 1);
  *buffer = CAR(cell)->number;
  return res;
}

/*
 * Process escapes in a list of characters.
 */

atom_t
lisp_process_escapes(const atom_t cell, const bool esc, const atom_t res)
{
  bool nesc = false;
  /*
   */
  if (cell == NIL) {
    X(cell);
    return res;
  }
  /*
   */
  atom_t nxt;
  atom_t car = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  /*
   * Process the character.
   */
  if (esc) {
    switch ((char)car->number) {
      case 'n' :
        X(car);
        car = lisp_make_char('\n');
        break;
      case 't' :
        X(car);
        car = lisp_make_char('\t');
        break;
      default:
        break;
    }
    nxt = lisp_append(res, car);
    nesc = false;
  }
  else if (car->number == '\\') {
    X(car);
    nesc = true;
    nxt = res;
  }
  else {
    nxt = lisp_append(res, car);
  }
  /*
   */
  return lisp_process_escapes(cdr, nesc, nxt);
}

/*
 * Get a timestamp in nanoseconds.
 */

uint64_t
lisp_timestamp()
{
  struct timespec ts = { 0 };
  clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
  return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}
