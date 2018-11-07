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
  LISP_FREE(B, C);
}

items(A) ::= quote(B).
{
  A = lisp_cons(B, NIL);
  LISP_FREE(B);
}

items(A) ::= items(B) quote(C).
{
  atom_t D = lisp_cons(C, NIL);
  A = lisp_conc(B, D);
  LISP_FREE(B, C, D);
}

quote(A) ::= item(B).
{
  A = B;
}

quote(A) ::= QUOTE item(B).
{
  atom_t Q = lisp_make_symbol(strdup("quote"));
  A = lisp_cons(Q, B);
  LISP_FREE(Q, B);
}

item(A) ::= NUMBER(B).
{
  A = lisp_make_number((int64_t)B);
}

item(A) ::= STRING(B).
{
  A = lisp_make_string((const char *)B);
}

item(A) ::= SYMBOL(B).
{
  A = lisp_make_symbol((char *)B);
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
