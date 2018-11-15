#include <mnml/lexer.h>
#include <mnml/slab.h>
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

#define UNPREFIX(_ts) (*(_ts) == '\'' || *(_ts) == '`' ? (_ts) + 1 : (_ts))

extern void parse_error();

static void
lisp_consume_token(const lexer_t lexer)
{
  if (lexer->depth == 0) {
    Parse(lexer->parser, 0, 0, lexer->consumer);
  }
}

%%{

machine minimal;

action err_prefix
{
  parse_error();
  fhold; fgoto purge;
}

action err_symbol
{
  parse_error();
  fhold; fgoto purge;
}

action tok_popen
{
  Parse(lexer->parser, POPEN, 0, NULL);
  lexer->depth += 1;
}

action tok_pclose
{
  Parse(lexer->parser, PCLOSE, 0, NULL);
  lexer->depth -= 1;
  lisp_consume_token(lexer);
}

action tok_dot
{
  Parse(lexer->parser, DOT, 0, NULL);
}

action tok_quote
{
  Parse(lexer->parser, QUOTE, 0, NULL);
}

action tok_backt
{
  Parse(lexer->parser, BACKTICK, 0, NULL);
}

action tok_number
{
  const char * start = UNPREFIX(ts);
  size_t len = te - start;
  char * val = (char *)alloca(len + 1);
  strncpy(val, start, len);
  val[len] = 0;
  int64_t value = strtoll(val, NULL, 10);
  Parse(lexer->parser, NUMBER, (void *)value, NULL);
  lisp_consume_token(lexer);
}

action tok_char
{
  const char * start = UNPREFIX(ts) + 1;
  const char * end = te - 1;
  size_t len = end - start;
  uint64_t val = *start;
  /*
   */
  if (len == 2) {
    switch (*(start + 1)) {
      case 'n' :
        val = '\n';
        break;
      case 't' :
        val = '\t';
        break;
      default:
        val = *(start + 1);
        break;
    }
  }
  /*
   */
  Parse(lexer->parser, CHAR, (void *)val, NULL);
  lisp_consume_token(lexer);
}

action tok_string
{
  const char * start = UNPREFIX(ts) + 1;
  const char * end = te - 1;
  size_t len = end - start;
  char * val = strndup(start, len);
  /*
   */
  Parse(lexer->parser, STRING, val, NULL);
  lisp_consume_token(lexer);
}

action tok_nil
{ 
  Parse(lexer->parser, C_NIL, 0, NULL);
  lisp_consume_token(lexer);
}

action tok_true
{ 
  Parse(lexer->parser, C_TRUE, 0, NULL);
  lisp_consume_token(lexer);
}

action tok_wildcard
{ 
  Parse(lexer->parser, C_WILDCARD, 0, NULL);
  lisp_consume_token(lexer);
}

action tok_symbol
{ 
  const char * start = UNPREFIX(ts);
  size_t len = te - start;
  MAKE_SYMBOL_DYNAMIC(sym, start, len);
  Parse(lexer->parser, SYMBOL, sym, NULL);
  lisp_consume_token(lexer);
}

popen   = '(';
pclose  = ')';
dot     = '.';
quote   = '\'';
backt   = '`';
number  = '-'? digit+;
char    = '\'' . (print - '\\' | '\\' . '\\') . '\'';
string  = '"' . ([^"] | '\\' '"')* . '"';
marks   = [~!@$%^&*_+\-={}\[\]:;|\\<>?,./];
symbol  = (alpha | marks) . (alnum | marks){,15} $!err_symbol;
comment = '#' . [^\n]*;

purge := any* %{ fgoto main; };

main := |*
  # 
  # Default rules.
  # 
  popen  => tok_popen;
  pclose => tok_pclose;
  dot    => tok_dot;
  number => tok_number;
  char   => tok_char;
  string => tok_string;
  "NIL"  => tok_nil;
  'T'    => tok_true;
  '_'    => tok_wildcard;
  symbol => tok_symbol;
  #
  # Quoted rules.
  #
  (quote >tok_quote | backt >tok_backt) . popen  $!err_prefix => tok_popen;
  (quote >tok_quote | backt >tok_backt) . number $!err_prefix => tok_number;
  (quote >tok_quote | backt >tok_backt) . string $!err_prefix => tok_string;
  (quote >tok_quote | backt >tok_backt) . "NIL"  $!err_prefix => tok_nil;
  (quote >tok_quote | backt >tok_backt) . 'T'    $!err_prefix => tok_true;
  (quote >tok_quote | backt >tok_backt) . '_'    $!err_prefix => tok_wildcard;
  (quote >tok_quote | backt >tok_backt) . symbol $!err_prefix => tok_symbol;
  #
  # Garbage.
  #
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
  lexer_t lexer = (lexer_t)malloc(sizeof(struct _lexer));
  %% write init;
  lexer->consumer = consumer;
  lexer->depth = 0;
  lexer->rem = 0;
  lexer->parser = ParseAlloc(local_malloc);
  return lexer;
}

void
lisp_destroy(const lexer_t lexer)
{
  ParseFree(lexer->parser, local_free);
  free(lexer);
}

void
lisp_parse(const lexer_t lexer, char * const str,
           const size_t len, const bool end)
{
  const char* p = str + lexer->rem;
  const char* pe = str + len;
  const char* eof = end ? pe : 0;
  %% write exec;
  /*
   * Update the local state when there is a prefix to save.
   */
  lexer->rem = 0;
  if (ts != 0) {
    lexer->rem = pe - ts;
    memmove(str, ts, lexer->rem);
    te = str + (te - ts);
    ts = str;
  }
}
