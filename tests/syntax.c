#include "primitives.h"
#include <syntax/lexer.h>

const char * testA =
"(prog # This is a program  \n"
"  (sub 1 1)                \n"
"  (add 2 2)                \n"
"  (quote . \"hello\")      \n"
"   (1 2 3 4)               \n"
"  '(1 2 . 3)               \n"
"  \"\"                     \n"
")                          \n";

const char * testB =
"() ()"
"     ";

void
syntax_error()
{

}

void
lisp_consumer(const cell_t cell)
{
  lisp_print(stdout, cell);
  lisp_free(cell);
}

int
main(const int argc, char ** const argv)
{
  /*
   * Create the lexer.
   */
  lexer_t lexer = lexer_create(lisp_consumer);
  /*
   * Run the tests.
   */
  lexer_parse(lexer, testA);
  lexer_parse(lexer, testB);
  /*
   * Clean-up.
   */
  lexer_destroy(lexer);
  return 0;
}
