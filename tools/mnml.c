#include <lisp/lexer.h>
#include <lisp/slab.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

typedef void (* stage_t)(const atom_t);

static bool keep_running = true;

static void
signal_handler(const int sigid)
{
  keep_running = false;
}

void
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

void
repl_parse_error_handler()
{
  write(1, "^ parse error\n", 14);
  if (CDR(CAR(ICHAN)) == NIL) {
    write(1, ": ", 2);
  }
}

void
stage_prompt(const atom_t cell)
{
  if (CDR(CAR(ICHAN)) == NIL) {
    write(1, ": ", 2);
  }
}

void
stage_newline(const atom_t cell)
{
  write(1, "> " , 2);
  lisp_prin(NIL, cell, true);
  write(1, "\n", 1);
}

void
stage_noop(const atom_t cell)
{

}

void
stage_push_io(const atom_t cell)
{
  PUSH_IO_CONTEXT(ICHAN, 0);
}

void
stage_pop_io(const atom_t cell)
{
  POP_IO_CONTEXT(ICHAN);
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
