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
  A = lisp_make_nil();
}

list(A) ::= POPEN items(B) PCLOSE.
{
  A = B;
}

list(A) ::= POPEN items(B) DOT prefix(C) PCLOSE.
{
  A = lisp_conc(B, C);
}

items(A) ::= prefix(B).
{
  A = lisp_cons(B, lisp_make_nil());
}

items(A) ::= items(B) prefix(C).
{
  A = lisp_append(B, C);
}

items(A) ::= items(B) TILDE prefix(C).
{
  atom_t nil = lisp_make_nil();
  C = lisp_eval(lexer->lisp, nil, C);
  A = lisp_conc(B, C);
  X(nil);
}

prefix(A) ::= item(B).
{
  A = B;
}

prefix(A) ::= CQUOTE item(B).
{
  A = lisp_cons(lisp_make_quote(), B);
}

prefix(A) ::= BACKTICK item(B).
{
  atom_t nil = lisp_make_nil();
  A = lisp_eval(lexer->lisp, nil, B);
  X(nil);
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
  A = lisp_make_nil();
}

item(A) ::= C_TRUE.
{
  A = lisp_make_true();
}

item(A) ::= C_WILDCARD.
{
  A = lisp_make_wildcard();
}

item(A) ::= list(B).
{
  A = B;
}
