#include <mnml/debug.h>
#include <mnml/lexer.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (* stage_t)(const atom_t);

static bool keep_running = true;

static void
signal_handler(const int sigid)
{
  keep_running = false;
}

static void
run(const stage_t pread, const stage_t peval, const stage_t post)
{
  atom_t input, result;
  while (keep_running) {
    pread(NIL);
    input = lisp_read(NIL, UP(NIL));
    peval(input);
    result = lisp_eval(NIL, input);
    post(result);
    X(result);
  }
}

static void
repl_parse_error_handler()
{
  write(1, "^ parse error\n", 14);
  if (CDR(CAR(ICHAN)) == NIL) {
    write(1, ": ", 2);
  }
}

static void
stage_prompt(const atom_t cell)
{
  if (CDR(CAR(ICHAN)) == NIL) {
    write(1, ": ", 2);
  }
}

static void
stage_newline(const atom_t cell)
{
  write(1, "> " , 2);
  lisp_prin(NIL, cell, true);
  write(1, "\n", 1);
}

static void
stage_noop(const atom_t cell)
{

}

static void
stage_push_io(const atom_t cell)
{
  PUSH_IO_CONTEXT(ICHAN, 0);
}

static void
stage_pop_io(const atom_t cell)
{
  POP_IO_CONTEXT(ICHAN);
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

int
main(const int argc, char ** const argv)
{
  int status = 0;
  lisp_init();
  /*
   */
  signal(SIGQUIT, signal_handler);
  signal(SIGTERM, signal_handler);
  /*
   * Build argv and env.
   */
  lisp_build_argv(argc, argv);
  lisp_build_env();
  /*
   */
  if (argc == 1) {
    lisp_set_parse_error_handler(repl_parse_error_handler);
    PUSH_IO_CONTEXT(ICHAN, 0);
    PUSH_IO_CONTEXT(OCHAN, 1);
    run(stage_prompt, stage_noop, stage_newline);
    POP_IO_CONTEXT(ICHAN);
    POP_IO_CONTEXT(OCHAN);
  }
  else {
    int fd = open(argv[1], O_RDONLY);
    if (fd >= 0) {
      PUSH_IO_CONTEXT(ICHAN, fd);
      PUSH_IO_CONTEXT(OCHAN, 1);
      run(stage_noop, stage_push_io, stage_pop_io);
      POP_IO_CONTEXT(ICHAN);
      POP_IO_CONTEXT(OCHAN);
      close(fd);
    }
  }
  /*
   */
  lisp_fini();
  status = slab.n_alloc - slab.n_free;
  return status;
}
