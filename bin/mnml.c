#include <mnml/debug.h>
#include <mnml/lexer.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (* stage_t)(const atom_t, const void * const data);

static bool keep_running = true;

static void
signal_handler(const int sigid)
{
  keep_running = false;
}

static atom_t
run(const stage_t pread, const stage_t peval, const stage_t pdone,
    const void * data)
{
  atom_t input, result = UP(NIL);
  while (keep_running) {
    X(result);
    pread(NIL, data);
    input = lisp_read(NIL, UP(NIL));
    if (input == NULL) {
      break;
    }
    peval(input, data);
    result = lisp_eval(NIL, input);
    pdone(result, data);
  }
  return result;
}

static void
repl_parse_error_handler()
{
  write(1, "^ parse error\n", 14);
  if (CDR(CAR(ICHAN)) == NIL) {
    fwrite(": ", 1, 2, stdout);
  }
}

static void
stage_prompt(const atom_t cell, const void * const data)
{
  if (CDR(CDR(CAR(ICHAN))) == NIL) {
    fwrite(": ", 1, 2, stdout);
  }
}

static void
stage_newline(const atom_t cell, const void * const data)
{
  fwrite("> " , 1, 2, stdout);
  lisp_prin(NIL, cell, true);
  fwrite("\n", 1, 1, stdout);
}

static void
stage_noop(const atom_t cell, const void * const data)
{

}

static void
lisp_build_argv(const int argc, char ** const argv)
{
  atom_t res = UP(NIL);
  /*
   * Build ARGV.
   */
  for (int i = 0; i < argc; i += 1) {
    atom_t str = lisp_make_string(argv[i], strlen(argv[i]));
    res = lisp_append(res, str);
  }
  /*
   * Set the variable if the list is not NIL.
   */
  if (!IS_NULL(res)) {
    MAKE_SYMBOL_STATIC(var, "ARGV", 4);
    atom_t key = lisp_make_symbol(var);
    GLOBALS = lisp_setq(GLOBALS, lisp_cons(key, res));
    X(key); X(res);
  }
  else {
    X(res);
  }
}

static void
lisp_build_env()
{
  extern char ** environ;
  atom_t res = UP(NIL);
  /*
   * Parse environ and build the variable list.
   */
  for (char ** p = environ; *p != NULL; p += 1) {
    char * n = strstr(*p, "=");
    if (n != NULL && *(n + 1) != 0) {
      size_t len = n - *p;
      atom_t key = lisp_make_string(*p, len);
      atom_t val = lisp_make_string(n + 1, strlen(n + 1));
      atom_t con = lisp_cons(key, val);
      res = lisp_append(res, con);
      X(key); X(val);
    }
  }
  /*
   * Set the variable if the list is not NIL.
   */
  if (!IS_NULL(res)) {
    MAKE_SYMBOL_STATIC(env, "ENV", 3);
    atom_t key = lisp_make_symbol(env);
    GLOBALS = lisp_setq(GLOBALS, lisp_cons(key, res));
    X(key); X(res);
  }
  else {
    X(res);
  }
}

static void
lisp_preload(const size_t n, ...)
{
  va_list args;
  va_start(args, n);
  for (size_t i = 0; i < n; i += 1) {
    const char * const symbol = va_arg(args, const char *);
    MAKE_SYMBOL_STATIC(s, symbol, LISP_GET_SYMBOL_LENGTH(symbol));
    atom_t cell = lisp_make_symbol(s);
    atom_t func = lisp_plugin_load(cell, NIL);
    if (IS_NULL(func)) {
      ERROR("Loading plugin %s failed", symbol);
    }
    X(cell); X(func);
  }
  va_end(args);
}

#define NUMARGS(...)  (sizeof((char *[]){__VA_ARGS__})/sizeof(char *))
#define PRELOAD(...)  lisp_preload(NUMARGS(__VA_ARGS__), __VA_ARGS__)

int
main(const int argc, char ** const argv)
{
  lisp_init();
  /*
   * Register system signals.
   */
  signal(SIGQUIT, signal_handler);
  signal(SIGTERM, signal_handler);
  /*
   * Get the current working dsirectory.
   */
  char cwd_buf[PATH_MAX];
  const char * const cwd = getcwd(cwd_buf, PATH_MAX);
  if (cwd == NULL) {
    fprintf(stderr, "Cannot get current directory: %s", strerror(errno));
    return __LINE__;
  }
  /*
   * Preload some basic functions if MNML_PRELOAD is not defined.
   */
  const char * PRELOAD = getenv("MNML_PRELOAD");
  if (PRELOAD == NULL) {
    PRELOAD(/* COMPARATORS */
            "=", "<>", "<", ">", "<=", ">=",
            /* ARITHMETICS */
            "+", "-", "*", "/", "%",
            /* LOGIC */
            "and", "or", "not",
            /* LIST */
            "car", "cdr", "conc", "cons", "list",
            /* SYMBOL */
            "<-", "\\", "def", "let", "setq", "sym",
            /* STRING AND CHARACTER */
            "chr", "str",
            /* CONTROL */
            "|>", "cond", "if", "match", "prog",
            /* PREDICATES */
            "chr?", "lst?", "nil?", "num?", "str?", "sym?", "tru?",
            /* I/O */
            "in", "out", "read", "readlines",
            /* PRINTERS */
            "prin", "prinl", "print", "printl",
            /* MISC */
            "eval", "load", "quit", "quote", "time"
             );
  }
  /*
   * Scan the PRELOAD list and load the symbols it contains.
   */
  else {
    FOR_EACH_TOKEN(PRELOAD, ",", entry, lisp_preload(1, entry));
  }
  /*
   * Build ARGV and ENV.
   */
  lisp_build_argv(argc, argv);
  lisp_build_env();
  /*
   */
  atom_t result;
  if (argc == 1) {
    lisp_set_parse_error_handler(repl_parse_error_handler);
    PUSH_IO_CONTEXT(ICHAN, stdin, cwd);
    PUSH_IO_CONTEXT(OCHAN, stdout, cwd);
    result = run(stage_prompt, stage_noop, stage_newline, cwd);
    POP_IO_CONTEXT(ICHAN);
    POP_IO_CONTEXT(OCHAN);
  }
  else {
    PUSH_IO_CONTEXT(OCHAN, stdout, cwd);
    result = lisp_load_file(argv[1]);
    POP_IO_CONTEXT(OCHAN);
  }
  /*
   * Compute the return status.
   */
  int status = IS_NULL(result) ? -1 : 0;
  X(result);
  /*
   */
  lisp_fini();
  return status;
}
