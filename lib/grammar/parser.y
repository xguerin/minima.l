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
  A = lisp_make_nil(lexer->lisp);
}

list(A) ::= POPEN items(B) PCLOSE.
{
  A = B;
}

list(A) ::= POPEN items(B) DOT prefix(C) PCLOSE.
{
  A = lisp_conc(lexer->lisp, B, C);
}

items(A) ::= prefix(B).
{
  A = lisp_cons(lexer->lisp, B, lisp_make_nil(lexer->lisp));
}

items(A) ::= items(B) prefix(C).
{
  A = lisp_append(lexer->lisp, B, C);
}

items(A) ::= items(B) TILDE prefix(C).
{
  atom_t nil = lisp_make_nil(lexer->lisp);
  C = lisp_eval(lexer->lisp, nil, C);
  A = lisp_conc(lexer->lisp, B, C);
  X(lexer->lisp->slab, nil);
}

prefix(A) ::= item(B).
{
  A = B;
}

prefix(A) ::= CQUOTE item(B).
{
  A = lisp_cons(lexer->lisp, lisp_make_quote(lexer->lisp), B);
}

prefix(A) ::= BACKTICK item(B).
{
  atom_t nil = lisp_make_nil(lexer->lisp);
  A = lisp_eval(lexer->lisp, nil, B);
  X(lexer->lisp->slab, nil);
}

item(A) ::= NUMBER(B).
{
  A = lisp_make_number(lexer->lisp, (int64_t)B);
}

item(A) ::= CHAR(B).
{
  A = lisp_make_char(lexer->lisp, (char)B);
}

item(A) ::= STRING(B).
{
  A = lisp_make_string(lexer->lisp, B, strlen(B));
  free(B);
}

item(A) ::= SCOPED_SYMBOL(B).
{
  A = lisp_make_scoped_symbol(lexer->lisp, (symbol_t)B);
  free(B);
}

item(A) ::= SYMBOL(B).
{
  A = lisp_make_symbol(lexer->lisp, (symbol_t)B);
  free(B);
}

item(A) ::= C_NIL.
{
  A = lisp_make_nil(lexer->lisp);
}

item(A) ::= C_TRUE.
{
  A = lisp_make_true(lexer->lisp);
}

item(A) ::= C_WILDCARD.
{
  A = lisp_make_wildcard(lexer->lisp);
}

item(A) ::= list(B).
{
  A = B;
}
