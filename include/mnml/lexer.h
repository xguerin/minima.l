#pragma once

#include <mnml/lisp.h>
#include <unistd.h>

/*
 * Lexer type.
 */
typedef struct _lexer
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
lexer_t;

/*
 * Lexer lifecycle.
 */
void lisp_create(const lisp_consumer_t consumer, lexer_t * const lexer);
void lisp_destroy(lexer_t * const lexer);

/*
 * Lexer parse.
 */
void lisp_parse(lexer_t * const lexer, char * const str,
                const size_t len, const bool end);

bool lisp_pending(const lexer_t * const lexer);

/*
 * Debug macros.
 */

#ifdef LISP_ENABLE_DEBUG

#define TRACE_LEXER(__s, ...) \
  FPRINTF(stderr, "! %s:%d: " __s "\n", __FUNCTION__, __LINE__, __VA_ARGS__)

#else

#define TRACE_LEXER(__s, ...)

#endif
