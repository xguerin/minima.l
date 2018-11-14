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
  size_t          rem;
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
size_t lisp_parse(const lexer_t lexer, const char * const str,
                  const size_t len, const bool end);

/*
 * Debug macros.
 */

#ifdef LISP_ENABLE_DEBUG

#define TRACE_LEXER(__s, ...) \
  FPRINTF(stderr, "! %s:%d: " __s "\n", __FUNCTION__, __LINE__, __VA_ARGS__)

#else

#define TRACE_LEXER(__s, ...)

#endif
