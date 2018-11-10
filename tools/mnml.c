#include <lisp/lexer.h>
#include <lisp/slab.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

typedef void (* stage_t)(const atom_t);

void
run(const stage_t pre, const stage_t post)
{
  atom_t input, result;
  for (;;) {
    pre(NIL);
    input = lisp_read(NIL, NIL);
    if (input == NIL) {
      X(input);
      break;
    }
    result = lisp_eval(NIL, input);
    post(result);
    X(result);
  }
}

void
repl_parse_error_handler()
{
  write(1, "^ parse error\n", 14);
  write(1, ": ", 2);
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
  lisp_prin(NIL, cell);
  write(1, "\n", 1);
}

void
stage_noop(const atom_t cell)
{

}

int
main(const int argc, char ** const argv)
{
  lisp_init();
  /*
   */
  if (argc == 1) {
    lisp_set_parse_error_handler(repl_parse_error_handler);
    run(stage_prompt, stage_newline);
  }
  else {
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) return __LINE__;
    /*
     * Push the new input context.
     */
    atom_t n0 = lisp_make_number(fd);
    atom_t in = lisp_cons(n0, NIL);
    X(n0);
    atom_t old = ICHAN;
    ICHAN = lisp_cons(in, old);
    X(old); X(in);
    TRACE_SEXP(ICHAN);
    /*
     */
    run(stage_noop, stage_noop);
    close(fd);
  }
  /*
   */
  lisp_fini();
  return 0;
}
