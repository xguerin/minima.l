%extra_argument { lisp_consumer_t consumer }

%token_type { void * }
%type list  { cell_t }
%type items { cell_t }
%type quote { cell_t }
%type item  { cell_t }

%include
{
#include <lisp/lisp.h>
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
  A = lisp_make_nil();
}

list(A) ::= POPEN items(B) PCLOSE.
{
  A = lisp_make_list(B);
  lisp_free(1, B);
}

list(A) ::= POPEN items(B) DOT item(C) PCLOSE.
{
  A = lisp_cons(B, C);
  lisp_free(2, B, C);
}

items(A) ::= quote(B).
{
  A = B;
}

items(A) ::= items(B) quote(C).
{
  cell_t p = B;
  while (GET_TYPE(p->cdr) == T_LIST) {
    p = GET_PNTR(cell_t, p->cdr);
  }
  SET_TYPE(p->cdr, T_LIST);
  SET_DATA(p->cdr, C);
  A = B;
}

quote(A) ::= item(B).
{
  A = B;
}

quote(A) ::= QUOTE item(B).
{
  cell_t Q = lisp_make_symbol("quote");
  A = lisp_cons(Q, B);
  lisp_free(2, Q, B);
}

item(A) ::= NUMBER(B).
{
  A = lisp_make_number((uint64_t)B);
}

item(A) ::= STRING(B).
{
  A = lisp_make_string((char *)B);
  free(B);
}

item(A) ::= SYMBOL(B).
{
  A = lisp_make_symbol((char *)B);
  free(B);
}

item(A) ::= list(B).
{
  A = B;
}
