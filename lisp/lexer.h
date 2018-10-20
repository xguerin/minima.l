#pragma once

#include "lisp.h"
#include <unistd.h>

/*
 * Lexer type.
 */
typedef struct _lexer_t
{
  int             cs;
  int             act;
  const char *    ts;
  const char *    te;
  size_t          depth;
  lisp_consumer_t consumer;
  void *          parser;
}
* lexer_t;

/*
 * Lexer lifecycle.
 */
lexer_t lisp_create(const lisp_consumer_t consumer);
void lisp_destroy(const lexer_t lexer);

/*
 * Lexer parse.
 */
void lisp_parse(const lexer_t lexer, const char * const str);
