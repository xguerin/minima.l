#include <mnml/debug.h>
#include <mnml/lexer.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*stage_t)(const lisp_t, const atom_t, const void* const data);

static bool keep_running = true;

#ifdef LISP_ENABLE_DEBUG
static void
signal_handler(const int sigid)
#else
static void
signal_handler(UNUSED const int sigid)
#endif
{
  TRACE("Caught signal: %d", sigid);
  keep_running = false;
}

/*
 * REPL.
 */

static atom_t
run(const lisp_t lisp, stage_t pread, const stage_t pdone, const void* data)
{
  atom_t nil = lisp_make_nil(lisp);
  atom_t input, result = lisp_make_nil(lisp);
  while (keep_running) {
    X(lisp, result);
    pread(lisp, nil, data);
    input = lisp_read(lisp, lisp_make_nil(lisp));
    if (input == NULL) {
      result = lisp_make_nil(lisp);
      break;
    }
    result = lisp_eval(lisp, nil, input);
    pdone(lisp, result, data);
  }
  X(lisp, nil);
  return result;
}

static void
repl_parse_error_handler(const lisp_t lisp)
{
  write(1, "^ parse error\n", 14);
  if (IS_NULL(CDR(CDR(CAR(lisp->ichan))))) {
    fwrite(": ", 1, 2, stdout);
  }
}

static void
stage_prompt(const lisp_t lisp, UNUSED const atom_t cell,
             UNUSED const void* const data)
{
  if (IS_NULL(CDR(CDR(CAR(lisp->ichan))))) {
    fwrite(": ", 1, 2, stdout);
  }
}

static void
stage_newline(const lisp_t lisp, const atom_t cell,
              UNUSED const void* const data)
{
  fwrite("> ", 1, 2, stdout);
  lisp_prin(lisp, cell, true);
  fwrite("\n", 1, 1, stdout);
}

/*
 * String evaluation.
 */

static atom_t PAIRS;

static void
lisp_push(const lisp_t lisp, const atom_t cell)
{
  PAIRS = lisp_append(lisp, PAIRS, cell);
}

static atom_t
lisp_pop(const lisp_t lisp)
{
  atom_t rslt = lisp_car(lisp, PAIRS);
  atom_t next = lisp_cdr(lisp, PAIRS);
  X(lisp, PAIRS);
  PAIRS = next;
  return rslt;
}

/*
 * Variable builders.
 */

static void
lisp_build_argv(const lisp_t lisp, const int argc, char** const argv)
{
  atom_t res = lisp_make_nil(lisp);
  /*
   * Build ARGV.
   */
  for (int i = 0; i < argc; i += 1) {
    atom_t str = lisp_make_string(lisp, argv[i], strlen(argv[i]));
    res = lisp_append(lisp, res, str);
  }
  /*
   * Set the variable if the list is not NIL.
   */
  if (!IS_NULL(res)) {
    MAKE_SYMBOL_STATIC(var, "ARGV");
    atom_t key = lisp_make_symbol(lisp, var);
    atom_t elt = lisp_cons(lisp, key, res);
    lisp->globals = lisp_setq(lisp, lisp->globals, elt);
  } else {
    X(lisp, res);
  }
}

static void
lisp_build_config(const lisp_t lisp)
{
  atom_t key, val, con;
  atom_t res = lisp_make_nil(lisp);
  /*
   * Add the version string.
   */
  MAKE_SYMBOL_STATIC(version, "VERSION");
  key = lisp_make_symbol(lisp, version);
  val = lisp_make_string(lisp, MNML_VERSION, strlen(MNML_VERSION));
  con = lisp_cons(lisp, key, val);
  res = lisp_cons(lisp, con, res);
  /*
   * Add the prefix.
   */
  MAKE_SYMBOL_STATIC(prefix, "PREFIX");
  key = lisp_make_symbol(lisp, prefix);
  val = lisp_make_string(lisp, lisp_prefix(), strlen(lisp_prefix()));
  con = lisp_cons(lisp, key, val);
  res = lisp_cons(lisp, con, res);
  /*
   * Add the compiler version.
   */
  MAKE_SYMBOL_STATIC(compver, "COMPVER");
  key = lisp_make_symbol(lisp, compver);
  val = lisp_make_string(lisp, MNML_COMPILER_VERSION,
                         strlen(MNML_COMPILER_VERSION));
  con = lisp_cons(lisp, key, val);
  res = lisp_cons(lisp, con, res);
  /*
   * Add the compiler ID.
   */
  MAKE_SYMBOL_STATIC(compid, "COMPID");
  key = lisp_make_symbol(lisp, compid);
  val = lisp_make_string(lisp, MNML_COMPILER_ID, strlen(MNML_COMPILER_ID));
  con = lisp_cons(lisp, key, val);
  res = lisp_cons(lisp, con, res);
  /*
   * Add the build timestamp.
   */
  MAKE_SYMBOL_STATIC(buildts, "BUILD_TS");
  key = lisp_make_symbol(lisp, buildts);
  val =
    lisp_make_string(lisp, MNML_BUILD_TIMESTAMP, strlen(MNML_BUILD_TIMESTAMP));
  con = lisp_cons(lisp, key, val);
  res = lisp_cons(lisp, con, res);
  /*
   * Set the variable if the list is not NIL.
   */
  MAKE_SYMBOL_STATIC(env, "CONFIG");
  key = lisp_make_symbol(lisp, env);
  atom_t elt = lisp_cons(lisp, key, res);
  lisp->globals = lisp_setq(lisp, lisp->globals, elt);
}

static void
lisp_build_env(const lisp_t lisp)
{
  extern char** environ;
  atom_t res = lisp_make_nil(lisp);
  /*
   * Parse environ and build the variable list.
   */
  for (char** p = environ; *p != NULL; p += 1) {
    char* n = strstr(*p, "=");
    if (n != NULL && *(n + 1) != 0) {
      size_t len = n - *p;
      atom_t key = lisp_make_string(lisp, *p, len);
      atom_t val = lisp_make_string(lisp, n + 1, strlen(n + 1));
      atom_t con = lisp_cons(lisp, key, val);
      res = lisp_append(lisp, res, con);
    }
  }
  /*
   * Set the variable if the list is not NIL.
   */
  if (!IS_NULL(res)) {
    MAKE_SYMBOL_STATIC(env, "ENV");
    atom_t key = lisp_make_symbol(lisp, env);
    atom_t elt = lisp_cons(lisp, key, res);
    lisp->globals = lisp_setq(lisp, lisp->globals, elt);
  } else {
    X(lisp, res);
  }
}

/*
 * Help.
 */

static void
lisp_help(const char* const name)
{
  fprintf(stderr, "Usage: %s [-b|-d|-h|-v] [-e EXPR | FILE.L]\n", name);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "\t-b: bare mode, don't load anything by default\n");
  fprintf(stderr, "\t-d: return non-zero status if slab is not empty\n");
  fprintf(stderr, "\t-e: evaluate EXPR\n");
  fprintf(stderr, "\t-h: print this help\n");
  fprintf(stderr, "\t-v: show Minima.l runtime information\n");
}

/*
 * Main.
 */

#if defined(__linux__)
#define GETOPT(_c, _v, _o) getopt(_c, _v, "+" _o)
#elif defined(__MACH__) || defined(__OpenBSD__)
#define GETOPT(_c, _v, _o) getopt(_c, _v, _o)
#else
#error "Operating system not supported"
#endif

int
main(const int argc, char** const argv)
{
  /*
   * Parse arguments.
   */
  int c;
  bool check_slab = false, load_defaults = true;
  char* expr = NULL;
  while ((c = GETOPT(argc, argv, "hvbde:")) != -1) {
    switch (c) {
      case 'b':
        load_defaults = false;
        break;
      case 'd':
        check_slab = true;
        break;
      case 'e':
        expr = optarg;
        break;
      case 'h':
        lisp_help(argv[0]);
        return 0;
      case 'v':
        fprintf(stdout, "%s\n", MNML_VERSION);
        return 0;
      default:
        lisp_help(argv[0]);
        return __LINE__;
    }
  }
  /*
   * Grab any extraneous options and use that as input files.
   */
  const char* filename = NULL;
  if (optind < argc) {
    filename = argv[optind];
  }
  /*
   * Register system signals.
   */
  signal(SIGQUIT, signal_handler);
  signal(SIGTERM, signal_handler);
  /*
   * Ignore other annoying signals.
   */
  signal(SIGPIPE, SIG_IGN);
  /*
   * Get the current working dsirectory.
   */
  char cwd_buf[PATH_MAX];
  const char* const cwd = getcwd(cwd_buf, PATH_MAX);
  if (cwd == NULL) {
    fprintf(stderr, "Cannot get current directory: %s", strerror(errno));
    return __LINE__;
  }
  /*
   * Create a lisp context.
   */
  slab_t slab = slab_new();
  lisp_t lisp = lisp_new(slab);
  /*
   * Setup the debug variables.
   */
#ifdef LISP_ENABLE_DEBUG
  lisp_debug_parse_flags();
#endif
  /*
   * Initialize the plugins.
   */
  if (!module_init(lisp)) {
    fprintf(stderr, "Minima.l engine initialization failed.\n");
    return __LINE__;
  }
  /*
   * Load defaults.
   */
  if (load_defaults) {
    module_load_defaults(lisp);
    lisp_build_argv(lisp, argc - optind, &argv[optind]);
    lisp_build_config(lisp);
    lisp_build_env(lisp);
  }
  /*
   */
  atom_t result;
  if (filename == NULL && expr == NULL) {
    lisp_set_parse_error_handler(repl_parse_error_handler);
    PUSH_IO_CONTEXT(lisp, lisp->ichan, stdin, cwd);
    PUSH_IO_CONTEXT(lisp, lisp->ochan, stdout, cwd);
    result = run(lisp, stage_prompt, stage_newline, cwd);
    POP_IO_CONTEXT(lisp, lisp->ichan);
    POP_IO_CONTEXT(lisp, lisp->ochan);
  } else if (filename == NULL) {
    /*
     * Setup the PAIRS to NIL.
     */
    PAIRS = lisp_make_nil(lisp);
    /*
     * Parse the expression.
     */
    lexer_t lexer = lexer_create(lisp, lisp_push);
    size_t len = strlen(expr);
    lexer_parse(lexer, expr, len, true);
    lexer_destroy(lexer);
    /*
     * Push the IO context.
     */
    PUSH_IO_CONTEXT(lisp, lisp->ichan, stdin, cwd);
    PUSH_IO_CONTEXT(lisp, lisp->ochan, stdout, cwd);
    /*
     * Evaluate the parsed expressions.
     */
    result = lisp_make_nil(lisp);
    while (keep_running) {
      atom_t nil = lisp_make_nil(lisp);
      atom_t car = lisp_pop(lisp);
      if (IS_NULL(car)) {
        X(lisp, nil, car);
        break;
      }
      X(lisp, result);
      result = lisp_eval(lisp, nil, car);
      X(lisp, nil);
    }
    /*
     * Pop the IO context.
     */
    POP_IO_CONTEXT(lisp, lisp->ichan);
    POP_IO_CONTEXT(lisp, lisp->ochan);
    /*
     * Clear the PAIRS.
     */
    X(lisp, PAIRS);
  } else {
    PUSH_IO_CONTEXT(lisp, lisp->ochan, stdout, cwd);
    result = lisp_load_file(lisp, filename);
    POP_IO_CONTEXT(lisp, lisp->ochan);
  }
  /*
   * Compute the return status.
   */
  int status = IS_NULL(result) ? -1 : 0;
  X(lisp, result);
  /*
   * Unload modules.
   */
  module_fini(lisp);
  /*
   * Clean-up the lisp context.
   */
  lisp_delete(lisp);
  /*
   * Check the slab, update the status, and destroy the slab.
   */
  if (check_slab && slab->n_alloc != slab->n_free) {
    status = -2;
  }
  slab_delete(slab);
  /*
   */
  return status;
}
