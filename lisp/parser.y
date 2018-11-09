%extra_argument { lisp_consumer_t consumer }

%token_type { void * }
%type list  { atom_t }
%type items { atom_t }
%type quote { atom_t }
%type item  { atom_t }

%include
{
#include "lisp.h"
#include <stdlib.h>

extern void syntax_error();
}

%syntax_error
{
  syntax_error();
}

%parse_failure
{
  syntax_error();
}

root ::= quote(A).
{
  consumer(A);
}

list(A) ::= POPEN PCLOSE.
{
  A = UP(NIL);
}

list(A) ::= POPEN items(B) PCLOSE.
{
  A = B;
}

list(A) ::= POPEN items(B) DOT quote(C) PCLOSE.
{
  A = lisp_conc(B, C);
  X(B); X(C);
}

items(A) ::= quote(B).
{
  A = lisp_cons(B, NIL);
  X(B);
}

items(A) ::= items(B) quote(C).
{
  atom_t D = lisp_cons(C, NIL);
  A = lisp_conc(B, D);
  X(B); X(C); X(D);
}

quote(A) ::= item(B).
{
  A = B;
}

quote(A) ::= QUOTE item(B).
{
  MAKE_SYMBOL(quote, "quote", 5);
  atom_t Q = lisp_make_symbol(quote);
  A = lisp_cons(Q, B);
  X(Q); X(B);
}

item(A) ::= NUMBER(B).
{
  A = lisp_make_number((int64_t)B);
}

item(A) ::= CHAR(B).
{
  A = lisp_make_char((char)B);
}

item(A) ::= STRING(B).
{
  atom_t res = UP(NIL);
  char * str = B;
  size_t len = strlen(str);
  for (size_t i = 0; i < len; i += 1) {
    atom_t c = lisp_make_char(str[len - i - 1]);
    atom_t n = lisp_cons(c, res);
    X(c); X(res);
    res = n;
  }
  A = res;
}

item(A) ::= SYMBOL(B).
{
  A = lisp_make_symbol((symbol_t)B);
}

item(A) ::= C_NIL.
{
  A = UP(NIL);
}

item(A) ::= C_TRUE.
{
  A = UP(TRUE);
}

item(A) ::= C_WILDCARD.
{
  A = UP(WILDCARD);
}

item(A) ::= list(B).
{
  A = B;
}
