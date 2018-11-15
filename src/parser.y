%extra_argument { lisp_consumer_t consumer }

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

list(A) ::= POPEN items(B) DOT prefix(C) PCLOSE.
{
  A = lisp_conc(B, C);
  X(B); X(C);
}

items(A) ::= prefix(B).
{
  A = lisp_cons(B, NIL);
  X(B);
}

items(A) ::= items(B) prefix(C).
{
  atom_t D = lisp_cons(C, NIL);
  A = lisp_conc(B, D);
  X(B); X(C); X(D);
}

prefix(A) ::= item(B).
{
  A = B;
}

prefix(A) ::= QUOTE item(B).
{
  MAKE_SYMBOL_STATIC(quote, "quote", 5);
  atom_t Q = lisp_make_symbol(quote);
  A = lisp_cons(Q, B);
  X(Q); X(B);
}

prefix(A) ::= BACKTICK item(B).
{
  A = lisp_eval(NIL, B);
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
  A = lisp_process_escapes(res, false, UP(NIL));
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
