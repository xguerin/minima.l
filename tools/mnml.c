#include <functions/functions.h>
#include <syntax/lexer.h>
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
lisp_consumer(const cell_t cell)
{
  cell_t result = NULL;
  if (lisp_eval(cell, &result)) {
    fprintf(stdout, "-> ");
    lisp_print(stdout, result);
    lisp_free(1, result);
  }
  else {
    fprintf(stdout, "! eval  error\n");
  }
  lisp_free(1, cell);
  show_prompt = true;
}

int
main(const int argc, char ** const argv)
{
  char * line = NULL;
  size_t linecap = 0;
  ssize_t linelen;
  lexer_t lexer = lexer_create(lisp_consumer);
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
    lexer_parse(lexer, line);
    goto loop;
  }
  /*
   * Clean-up.
   */
  printf("\n");
  free(line);
  lexer_destroy(lexer);
  return 0;
}
