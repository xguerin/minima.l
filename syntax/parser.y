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
  A = NULL;
}

list(A) ::= POPEN items(B) PCLOSE.
{
  A = B;
}

list(A) ::= POPEN items(B) DOT item(C) PCLOSE.
{
  cell_t p = B;
  while (GET_TYPE(p->cdr) == T_LIST) p = GET_PNTR(cell_t, p->cdr);
  SET_TYPE(p->cdr, GET_TYPE(C->car));
  SET_DATA(p->cdr, GET_DATA(C->car));
  A = B;
}

items(A) ::= quote(B).
{
  A = B;
}

items(A) ::= items(B) quote(C).
{
  cell_t p = B;
  while (GET_TYPE(p->cdr) == T_LIST) p = GET_PNTR(cell_t, p->cdr);
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
  /*
   * Build the QUOTE cell.
   */
  cell_t Q;
  posix_memalign((void **)&Q, 16, sizeof(struct _cell_t));
  memset(Q, 0, sizeof(struct _cell_t));
  SET_TYPE(Q->car, T_SYMBOL_INLINE);
  SET_SYMB(Q->car, "quote");
  SET_TYPE(Q->cdr, GET_TYPE(B->car));
  SET_DATA(Q->cdr, GET_DATA(B->car));
  /*
   * Add the QUOTE cell into the list.
   */
  posix_memalign((void **)&A, 16, sizeof(struct _cell_t));
  memset(A, 0, sizeof(struct _cell_t));
  SET_TYPE(A->car, T_LIST);
  SET_DATA(A->car, Q);
  SET_TYPE(A->cdr, T_NIL);
}

item(A) ::= NUMBER(B).
{
  posix_memalign((void **)&A, 16, sizeof(struct _cell_t));
  memset(A, 0, sizeof(struct _cell_t));
  SET_TYPE(A->car, T_NUMBER);
  SET_NUMB(A->car, (uint64_t)B);
  SET_TYPE(A->cdr, T_NIL);
}

item(A) ::= STRING(B).
{
  posix_memalign((void **)&A, 16, sizeof(struct _cell_t));
  memset(A, 0, sizeof(struct _cell_t));
  SET_TYPE(A->car, T_STRING);
  SET_DATA(A->car, (char *)B);
  SET_TYPE(A->cdr, T_NIL);
}

item(A) ::= SYMBOL(B).
{
  posix_memalign((void **)&A, 16, sizeof(struct _cell_t));
  memset(A, 0, sizeof(struct _cell_t));
  SET_TYPE(A->car, T_SYMBOL);
  SET_DATA(A->car, (char *)B);
  SET_TYPE(A->cdr, T_NIL);
}

item(A) ::= list(B).
{
  posix_memalign((void **)&A, 16, sizeof(struct _cell_t));
  memset(A, 0, sizeof(struct _cell_t));
  SET_TYPE(A->car, T_LIST);
  SET_DATA(A->car, (char *)B);
  SET_TYPE(A->cdr, T_NIL);
}
