#include <mnml/debug.h>
#include <mnml/maker.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <dirent.h>
#include <dlfcn.h>
#include <libgen.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>

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

const char *
lisp_prefix()
{
  static bool is_set = false;
  static char prefix[PATH_MAX] = { 0 };
  Dl_info libInfo;
  /*
   * Compute this library path.
   */
  if (!is_set && dladdr(&lisp_prefix, &libInfo) != 0) {
    char buffer[PATH_MAX] = { 0 };
#if defined(__OpenBSD__)
    strlcpy(buffer, libInfo.dli_fname, 4096);
#else
    strcpy(buffer, libInfo.dli_fname);
#endif
    const char * dname = dirname(dirname(buffer));
    strcpy(prefix, dname);
    is_set = true;
  }
  /*
   * Return the result.
   */
  return prefix;
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
  lisp_debug_parse_flags();
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
 * Merge define-site and call-site closures. Both closures is consumed.
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
 * Return true if a cell is a string.
 */

bool
lisp_is_string(const atom_t cell)
{
  /*
   * Default case.
   */
  if (IS_NULL(cell)) {
    return true;
  }
  /*
   * Must be a list.
   */
  if (!IS_PAIR(cell)) {
    return false;
  }
  /*
   * Check CAR.
   */
  if (!IS_CHAR(cell->pair.car)) {
    return false;
  }
  /*
   * Recurse over CDR.
   */
  return lisp_is_string(cell->pair.cdr);
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
#if defined(__OpenBSD__)
  clock_gettime(CLOCK_MONOTONIC, &ts);
#else
  clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
#endif
  return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

const char *
lisp_get_fullpath(const char * const filepath, char * const buffer)
{
  char expn_buf[PATH_MAX];
  char absl_buf[PATH_MAX];
  /*
   * Expand the path.
   */
  if (strncmp(filepath, "@lib", 4) == 0) {
    strcpy(expn_buf, lisp_prefix());
    strcat(expn_buf, "/lisp");
    strcat(expn_buf, &filepath[4]);
  } else {
    strcpy(expn_buf, filepath);
  }
  /*
   * If the expanded path is not absolute, prepend the current CWD.
   */
  if (expn_buf[0] != '/' && !IS_NULL(ICHAN)) {
    lisp_make_cstring(CAR(CDR(CAR(ICHAN))), absl_buf, PATH_MAX, 0);
    strcat(absl_buf, "/");
    strcat(absl_buf, expn_buf);
  } else {
    strcpy(absl_buf, expn_buf);
  }
  /*
   * Get the fullpath.
   */
  const char * path = realpath(absl_buf, buffer);
  if (path == NULL) {
    ERROR("Cannot get realpath for %s", absl_buf);
    return NULL;
  }
  /*
   * Return the path.
   */
  return path;
}

atom_t
lisp_load_file(const char * const filepath)
{
  char path_buf[PATH_MAX];
  char dirn_buf[PATH_MAX];
  char curd_buf[PATH_MAX];
  /*
   * Get the fullpath for the file.
   */
  const char * path = lisp_get_fullpath(filepath, path_buf);
  if (path == NULL) {
    ERROR("Cannot get the full path for %s", filepath);
    return UP(NIL);
  }
  /*
   * Grab the directory of the file.
   */
  strcpy(dirn_buf, path);
  const char * dir = dirname(dirn_buf);
  if (dir == NULL) {
    ERROR("Cannot get directory for %s", path);
    return UP(NIL);
  }
  /*
   * Get the current working directory.
   */
  const char * const cwd = getcwd(curd_buf, PATH_MAX);
  if (cwd == NULL) {
    ERROR("Cannot get CWD for %s", path);
    return UP(NIL);
  }
  /*
   * Open the file.
   */
  FILE* handle = fopen(path, "r");
  if (handle == NULL) {
    ERROR("Cannot open %s", path);
    return UP(NIL);
  }
  /*
   * Push the context.
   */
  TRACE("Loading %s", path);
  PUSH_IO_CONTEXT(ICHAN, handle, dir);
  /*
   * Load all the entries
   */
  atom_t input, res = UP(NIL);
  while ((input = lisp_read(NIL, UP(NIL))) != NULL) {
    X(res);
    res = lisp_eval(NIL, input);
  }
  /*
   * Pop the context and return the value.
   */
  POP_IO_CONTEXT(ICHAN);
  fclose(handle);
  return res;
}
