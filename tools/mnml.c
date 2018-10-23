#include <lisp/functions.h>
#include <lisp/lexer.h>
#include <lisp/slab.h>
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
  cell_t result = lisp_eval(NIL, cell);
  fprintf(stdout, "> ");
  lisp_print(stdout, result);
  LISP_FREE(result);
#ifdef LISP_ENABLE_DEBUG
  fprintf(stdout, "D %ld\n", slab.n_alloc - slab.n_free);
#endif
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
  LISP_FREE(NIL);
  free(line);
  lisp_destroy(lexer);
  return 0;
}
