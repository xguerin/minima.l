#include <lisp/functions.h>
#include <lisp/lexer.h>
#include <stdio.h>
#include <stdlib.h>

static bool show_prompt = true;

void
syntax_error()
{
  fprintf(stdout, "! syntax error\n");
  show_prompt = true;
}

void
lisp_repl_consumer(const cell_t cell)
{
  cell_t result = lisp_eval(cell);
  size_t allocs = lisp_stats_get_alloc();
  size_t frees = lisp_stats_get_free();
  fprintf(stdout, "[%ld, %ld, %ld]-> ", allocs, frees, allocs - frees);
  lisp_print(stdout, result);
  lisp_free(1, result);
  show_prompt = true;
}

int
main(const int argc, char ** const argv)
{
  char * line = NULL;
  size_t linecap = 0;
  ssize_t linelen;
  lexer_t lexer = lisp_create(lisp_repl_consumer);
  /*
   * Run the parser loop.
   */
  lisp_function_register_all();
loop:
  if (show_prompt && lexer->depth == 0) {
    fprintf(stdout, ": ");
    fflush(stdout);
    show_prompt = false;
  }
  linelen = getline(&line, &linecap, stdin);
  if (linelen > 0) {
    lisp_parse(lexer, line);
    goto loop;
  }
  /*
   * Clean-up.
   */
  printf("\n");
  free(line);
  lisp_destroy(lexer);
  return 0;
}
