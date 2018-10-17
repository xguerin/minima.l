#include <syntax/lexer.h>
#include <stdio.h>
#include <stdlib.h>

static bool show_prompt = true;

void
syntax_error()
{
  show_prompt = true;
}

void
lisp_consumer(const cell_t cell)
{
  lisp_print(cell);
  lisp_free(cell);
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
loop:
  if (show_prompt) {
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
