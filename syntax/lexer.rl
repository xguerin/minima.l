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
  char * result = (char *)malloc(len + 1);
  memset(result, 0, len + 1);
  strncpy(result, s, len);
  return result;
}

static uint64_t
get_number(const char * const s, const char * const e)
{
  char * str = get_string(s, e);
  uint64_t result = strtoll(str, NULL, 10);
  free(str);
  return result;
}

%%{

machine Minimal;

action tok_popen  { Parse(lexer->parser, POPEN, 0, NULL); }
action tok_pclose { Parse(lexer->parser, PCLOSE, 0, NULL); }
action tok_dot    { Parse(lexer->parser, DOT, 0, NULL); }
action tok_quote  { Parse(lexer->parser, QUOTE, 0, NULL); }

action tok_number
{
  uint64_t value = get_number(ts, te);
  Parse(lexer->parser, NUMBER, (void *)value, NULL);
}

action tok_string
{
  Parse(lexer->parser, STRING, get_string(ts+1, te-1), NULL);
}

action tok_symbol
{ 
  Parse(lexer->parser, SYMBOL, get_string(ts, te), NULL);
}

popen  = '(';
pclose = ')';
dot    = '.';
quote  = '\'';
number = [1-9] digit*;
string = '"' [^"]* '"';
symbol = alpha alnum*;
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
lexer_create()
{
  lexer_t lexer = (lexer_t)malloc(sizeof(struct _lexer_t));
  %% write init;
  lexer->parser = ParseAlloc(malloc);
  return lexer;
}

void
lexer_destroy(const lexer_t lexer)
{
  ParseFree(lexer->parser, free);
  free(lexer);
}

cell_t
lexer_parse(const lexer_t lexer, const char * const str, const size_t len)
{
  const char* p = str;
  const char* pe = str + len;
  const char* eof = pe;
  cell_t result = NULL;
  %% write exec;
  Parse(lexer->parser, 0, 0, &result);
  return result;
}
