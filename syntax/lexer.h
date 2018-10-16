#pragma once

#include <lisp/lisp.h>
#include <unistd.h>

/*
 * Lexer type.
 */
typedef struct _lexer_t
{
  int           cs;
  int           act;
  const char *  ts;
  const char *  te;
  void *        parser;
}
* lexer_t;

/*
 * Lexer lifecycle.
 */
lexer_t lexer_create();
void lexer_destroy(const lexer_t lexer);

/*
 * Lexer parse.
 */
cell_t lexer_parse(const lexer_t lexer, const char * const str, const size_t len);
