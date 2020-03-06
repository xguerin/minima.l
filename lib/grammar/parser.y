%extra_argument { lexer_t lexer }

%token_type { void * }
%type list  { atom_t }
%type items { atom_t }
%type prefix { atom_t }
%type item  { atom_t }

%include
{
#include <mnml/lisp.h>
#include <mnml/utils.h>
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

root ::= prefix(A).
{
  lexer->consumer(lexer->lisp, A);
}

list(A) ::= POPEN PCLOSE.
{
  A = UP(NIL);
}

list(A) ::= POPEN items(B) PCLOSE.
{
  A = B;
}

list(A) ::= POPEN items(B) DOT prefix(C) PCLOSE.
{
  A = lisp_conc(B, C);
  X(B, C);
}

items(A) ::= prefix(B).
{
  A = lisp_cons(B, NIL);
  X(B);
}

items(A) ::= items(B) prefix(C).
{
  A = lisp_append(B, C);
}

items(A) ::= items(B) TILDE prefix(C).
{
  C = lisp_eval(lexer->lisp, NIL, C);
  A = lisp_conc(B, C);
  X(B, C);
}

prefix(A) ::= item(B).
{
  A = B;
}

prefix(A) ::= CQUOTE item(B).
{
  A = lisp_cons(QUOTE, B);
  X(B);
}

prefix(A) ::= BACKTICK item(B).
{
  A = lisp_eval(lexer->lisp, NIL, B);
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
  A = lisp_make_string(B, strlen(B));
  free(B);
}

item(A) ::= SYMBOL(B).
{
  A = lisp_make_symbol((symbol_t)B);
  free(B);
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
