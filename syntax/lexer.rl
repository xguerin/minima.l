#include <syntax/lexer.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "parser.c"

#define cs  lexer->cs
#define act lexer->act
#define ts  lexer->ts
#define te  lexer->te

static char *
get_string(const char * const s, const char * const e)
{
  size_t len = e - s;
  char * result = NULL;
  posix_memalign((void **)&result, 16, len + 1);
  memset(result, 0, len + 1);
  strncpy(result, s, len);
  return result;
}

static uint64_t
get_number(const char * const s, const char * const e)
{
  size_t len = e - s + 1;
  char * val = (char *)alloca(len);
  strncpy(val, s, len);
  return strtoll(val, NULL, 10);
}

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
  uint64_t value = get_number(ts, te);
  Parse(lexer->parser, NUMBER, (void *)value, NULL);
  if (lexer->depth == 0) {
    Parse(lexer->parser, 0, 0, lexer->consumer);
  }
}

action tok_string
{
  Parse(lexer->parser, STRING, get_string(ts+1, te-1), NULL);
  if (lexer->depth == 0) {
    Parse(lexer->parser, 0, 0, lexer->consumer);
  }
}

action tok_symbol
{ 
  Parse(lexer->parser, SYMBOL, get_string(ts, te), NULL);
  if (lexer->depth == 0) {
    Parse(lexer->parser, 0, 0, lexer->consumer);
  }
}

popen  = '(';
pclose = ')';
dot    = '.';
quote  = '\'';
number = [1-9] digit*;
string = '"' [^"]* '"';
marks  = [~!@#$%^&*_+\-={}\[\]:;|\\<>?,./];
symbol = (alpha | marks) (alnum | marks)*;
comment = '#' [^\n]*;

main := |*
  popen  => tok_popen;
  pclose => tok_pclose;
  dot    => tok_dot;
  quote  => tok_quote;
  number => tok_number;
  string => tok_string;
  symbol => tok_symbol;
  comment;
  space;
*|;

}%%

%% write data;

lexer_t
lexer_create(const lisp_consumer_t consumer)
{
  lexer_t lexer = (lexer_t)malloc(sizeof(struct _lexer_t));
  %% write init;
  lexer->consumer = consumer;
  lexer->depth = 0;
  lexer->parser = ParseAlloc(malloc);
  return lexer;
}

void
lexer_destroy(const lexer_t lexer)
{
  ParseFree(lexer->parser, free);
  free(lexer);
}

void
lexer_parse(const lexer_t lexer, const char * const str)
{
  const char* p = str;
  const char* pe = str + strlen(str);
  const char* eof = pe;
  %% write exec;
}
