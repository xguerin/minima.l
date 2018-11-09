#include <lisp/lexer.h>
#include <lisp/slab.h>
#include <stdio.h>
#include <stdlib.h>

static bool show_prompt = true;

typedef void (* stage_t)(const lexer_t lexer);

void
syntax_error_handler()
{
  fprintf(stdout, "! syntax error\n");
  show_prompt = true;
}

static void
lisp_repl_consumer(const atom_t cell)
{
  atom_t result = lisp_eval(NIL, cell);
  TRACE_SEXP(result);
  fprintf(stdout, "> ");
  lisp_print(stdout, result);
  X(result);
  TRACE("D %ld", slab.n_alloc - slab.n_free);
  show_prompt = true;
}

static void
lisp_file_consumer(const atom_t cell)
{
  atom_t result = lisp_eval(NIL, cell);
  X(result);
}

void
stage_prompt(const lexer_t lexer)
{
  if (show_prompt && lexer->depth == 0) {
    fprintf(stdout, ": ");
    fflush(stdout);
    show_prompt = false;
  }
}

void
stage_noop(const lexer_t lexer)
{

}

void
stage_newline(const lexer_t lexer)
{
  fprintf(stdout, "\n");
}

static void
process(const lisp_consumer_t cons, FILE * const file, const stage_t pre,
        const stage_t post)
{
  char * line = NULL;
  size_t linecap = 0;
  ssize_t linelen;
  /*
   * Create the parser and register all functions.
   */
  lexer_t lexer = lisp_create(cons);
  lisp_set_syntax_error_handler(syntax_error_handler);
  /*
   * Process input lines.
   */
B:pre(lexer);
  linelen = getline(&line, &linecap, file);
  if (linelen > 0) {
    lisp_parse(lexer, line);
    goto B;
  }
  /*
   * Clean-up.
   */
  free(line);
  post(lexer);
  lisp_destroy(lexer);
}

int
main(const int argc, char ** const argv)
{
  /*
   * REPL mode.
   */
  if (argc == 1)  {
    process(lisp_repl_consumer, stdin, stage_prompt, stage_newline);
  }
  /*
   * FILE processing mode.
   */
  else {
    FILE * file = fopen(argv[1], "r");
    if (file == NULL) {
      fprintf(stdout, "Cannot open file: \"%s\"\n", argv[1]);
      return __LINE__;
    }
    /*
     * Run the parser.
     */
    process(lisp_file_consumer, file, stage_noop, stage_noop);
    fclose(file);
  }
  return 0;
}
