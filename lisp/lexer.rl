#include "lexer.h"
#include "slab.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "parser.h"
#include "parser.c"

#define cs  lexer->cs
#define act lexer->act
#define ts  lexer->ts
#define te  lexer->te

%%{

machine minimal;

action tok_popen
{
  Parse(lexer->parser, POPEN, 0, NULL);
  lexer->depth += 1;
}

action tok_pclose
{
  Parse(lexer->parser, PCLOSE, 0, NULL);
  lexer->depth -= 1;
  if (lexer->depth == 0) {
    Parse(lexer->parser, 0, 0, lexer->consumer);
  }
}

action tok_dot
{
  Parse(lexer->parser, DOT, 0, NULL);
}

action tok_quote
{
  Parse(lexer->parser, QUOTE, 0, NULL);
}

action tok_number
{
  size_t len = te - ts + 1;
  char * val = (char *)alloca(len);
  strncpy(val, ts, len);
  int64_t value = strtoll(val, NULL, 10);
  /*
   */
  Parse(lexer->parser, NUMBER, (void *)value, NULL);
  if (lexer->depth == 0) {
    Parse(lexer->parser, 0, 0, lexer->consumer);
  }
}

action tok_string
{
  const char * start = ts + 1, * end = te - 1;
  size_t len = end - start;
  char * val = strndup(start, len);
  /*
   */
  Parse(lexer->parser, STRING, val, NULL);
  if (lexer->depth == 0) {
    Parse(lexer->parser, 0, 0, lexer->consumer);
  }
}

action tok_nil
{ 
  Parse(lexer->parser, C_NIL, 0, NULL);
  if (lexer->depth == 0) {
    Parse(lexer->parser, 0, 0, lexer->consumer);
  }
}

action tok_true
{ 
  Parse(lexer->parser, C_TRUE, 0, NULL);
  if (lexer->depth == 0) {
    Parse(lexer->parser, 0, 0, lexer->consumer);
  }
}

action tok_wildcard
{ 
  Parse(lexer->parser, C_WILDCARD, 0, NULL);
  if (lexer->depth == 0) {
    Parse(lexer->parser, 0, 0, lexer->consumer);
  }
}

action tok_symbol
{ 
  size_t len = te - ts;
  MAKE_SYMBOL(sym, ts, len);
  /*
   */
  Parse(lexer->parser, SYMBOL, sym, NULL);
  if (lexer->depth == 0) {
    Parse(lexer->parser, 0, 0, lexer->consumer);
  }
}

popen  = '(';
pclose = ')';
dot    = '.';
quote  = '\'';
number = '-'? digit+;
string = '"' [^"]* '"';
marks  = [~!@$%^&*_+\-={}\[\]:;|\\<>?,./];
symchr = (alpha | marks);
symbol = symchr{1,16};
comment = '#' [^\n]*;

main := |*
  popen  => tok_popen;
  pclose => tok_pclose;
  dot    => tok_dot;
  quote  => tok_quote;
  number => tok_number;
  string => tok_string;
  "NIL"  => tok_nil;
  'T'    => tok_true;
  '_'    => tok_wildcard;
  symbol => tok_symbol;
  comment;
  space;
*|;

}%%

%% write data;

typedef struct _region
{
  size_t size;
  uint8_t data[];
}
* region_t;

static void *
local_malloc(const size_t size)
{
  void * data = mmap(NULL, size + sizeof(struct _region),
                     PROT_READ | PROT_WRITE,
                     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  TRACE_LEXER("allocate region for %luB at 0x%lx", size, (uintptr_t)data);
  memset(data, 0, size);
  region_t region = (region_t)data;
  region->size = size;
  return region->data;
}

static void
local_free(void * const addr)
{
  region_t region = (region_t)(addr - sizeof(struct _region));
  TRACE_LEXER("free 0x%lx", (uintptr_t)region);
  munmap(region, region->size);
}

lexer_t
lisp_create(const lisp_consumer_t consumer)
{
  lisp_slab_allocate();
  /*
   * Create the constants.
   */
  lisp_make_nil();
  lisp_make_true();
  lisp_make_wildcard();
  /*
   * Create the GLOBALS and the lexer.
   */
  GLOBALS = UP(NIL);
  /*
   */
  lexer_t lexer = (lexer_t)malloc(sizeof(struct _lexer_t));
  %% write init;
  lexer->consumer = consumer;
  lexer->depth = 0;
  lexer->parser = ParseAlloc(local_malloc);
  return lexer;
}

void
lisp_destroy(const lexer_t lexer)
{
  ParseFree(lexer->parser, local_free);
  free(lexer);
  X(GLOBALS); X(WILDCARD); X(TRUE); X(NIL);
  TRACE("D %ld", slab.n_alloc - slab.n_free);
  LISP_COLLECT();
  lisp_slab_destroy();
}

void
lisp_parse(const lexer_t lexer, const char * const str)
{
  const char* p = str;
  const char* pe = str + strlen(str);
  const char* eof = pe;
  %% write exec;
}
