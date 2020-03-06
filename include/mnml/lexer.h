#pragma once

#include <mnml/lisp.h>
#include <unistd.h>

/*
 * Consumer type.
 */

typedef void (*lisp_consumer_t)(const lisp_t lisp, const atom_t);

/*
 * Lexer type.
 */
typedef struct _lexer
{
  int cs;
  int act;
  const char* ts;
  const char* te;
  size_t depth;
  size_t rem;
  lisp_t lisp;
  lisp_consumer_t consumer;
  void* parser;
} * lexer_t;

/*
 * Lexer lifecycle.
 */
lexer_t lexer_create(const lisp_t lisp, const lisp_consumer_t consumer);
void lexer_destroy(const lexer_t lexer);

/*
 * Lexer parse.
 */
void lexer_parse(const lexer_t lexer, char* const str, const size_t len,
                 const bool end);

bool lexer_pending(const lexer_t lexer);

/*
 * Debug macros.
 */

#ifdef LISP_ENABLE_DEBUG

#define TRACE_LEXER(__s, ...) \
  FPRINTF(stderr, "! %s:%d: " __s "\n", __FUNCTION__, __LINE__, __VA_ARGS__)

#else

#define TRACE_LEXER(__s, ...)

#endif

// vim: tw=80:sw=2:ts=2:sts=2:et
