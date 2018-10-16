#include "primitives.h"
#include <syntax/lexer.h>

const char * text =
"(prog # This is a program  \n"
"  (sub 1 1)                \n"
"  (add 2 2)                \n"
"  (quote . \"hello\")      \n"
"   (1 2 3 4)               \n"
"  '(1 2 . 3)               \n"
"  \"\"                     \n"
")                          \n";

int
main(const int argc, char ** const argv)
{
  lexer_t lexer = lexer_create();
  cell_t cell = lexer_parse(lexer, text, strlen(text));
  assert(cell != NULL);
  lisp_print(cell);
  lexer_destroy(lexer);
  return 0;
}
