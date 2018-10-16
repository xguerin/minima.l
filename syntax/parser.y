%extra_argument { cell_t * result }

%token_type { void * }
%type list  { cell_t }
%type items { cell_t }
%type quote { cell_t }
%type item  { cell_t }

%include
{
#include <stdlib.h>
#include <lisp/lisp.h>
}

%syntax_error  { }
%parse_failure { }

root ::= list(A).
{
  *result = A;
}

list(A) ::= POPEN items(B) PCLOSE.
{
  A = B;
}

list(A) ::= POPEN items(B) DOT item(C) PCLOSE.
{
  cell_t p = B;
  while (p->cdr.type == T_CELL) p = p->cdr.value.cell;
  p->cdr.type = C->car.type;
  p->cdr.value = C->car.value;
  A = B;
  free(C);
}

items(A) ::= .
{
  A = NULL;
}

items(A) ::= quote(B) items(C).
{
  if (C == NULL) {
    B->cdr.type = T_NIL;
    B->cdr.value.cell = NULL;
  }
  else {
    B->cdr.type = T_CELL;
    B->cdr.value.cell = C;
  }
  A = B;
}

quote(A) ::= item(B).
{
  A = B;
}

quote(A) ::= QUOTE item(B).
{
  A = B;
}

item(A) ::= NUMBER(B).
{
  A = (cell_t)malloc(sizeof(struct _cell_t));
  A->car.type = T_NUMBER;
  A->car.value.number = (uint64_t)B;
  A->cdr.type = T_NIL;
}

item(A) ::= STRING(B).
{
  A = (cell_t)malloc(sizeof(struct _cell_t));
  A->car.type = T_STRING;
  A->car.value.string = (char *)B;
  A->cdr.type = T_NIL;
}

item(A) ::= SYMBOL(B).
{
  A = (cell_t)malloc(sizeof(struct _cell_t));
  A->car.type = T_SYMBOL;
  A->car.value.symbol = (char *)B;
  A->cdr.type = T_NIL;
}

item(A) ::= list(B).
{
  A = (cell_t)malloc(sizeof(struct _cell_t));
  A->car.type = T_CELL;
  A->car.value.cell = B;
  A->cdr.type = T_NIL;
}
