#include <mnml/debug.h>
#include <mnml/lisp.h>
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
  MNML_DEBUG        = getenv("MNML_DEBUG")        != NULL;
  MNML_VERBOSE_CONS = getenv("MNML_VERBOSE_CONS") != NULL;
  MNML_VERBOSE_RC   = getenv("MNML_VERBOSE_RC")   != NULL;
  MNML_VERBOSE_SLOT = getenv("MNML_VERBOSE_SLOT") != NULL;
  MNML_VERBOSE_SLAB = getenv("MNML_VERBOSE_SLAB") != NULL;
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
  atom_t sym = lisp_car(elt);
  atom_t val = lisp_cdr(elt);
  X(elt);
  atom_t res = lisp_setq(call, sym, val);
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

/*
 * Singleton constructors.
 */

void
lisp_make_nil()
{
  atom_t R = lisp_allocate();
  R->type = T_NIL;
  R->refs = 1;
  TRACE_SEXP(R);
  NIL = R;
}

void
lisp_make_true()
{
  atom_t R = lisp_allocate();
  R->type = T_TRUE;
  R->refs = 1;
  TRACE_SEXP(R);
  TRUE = R;
}

void
lisp_make_quote()
{
  MAKE_SYMBOL_STATIC(quote, "quote", 5);
  atom_t R = lisp_make_symbol(quote);
  TRACE_SEXP(R);
  QUOTE = R;
}

void
lisp_make_wildcard()
{
  atom_t R = lisp_allocate();
  R->type = T_WILDCARD;
  R->refs = 1;
  TRACE_SEXP(R);
  WILDCARD = R;
}
