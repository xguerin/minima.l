#include "utils.h"
#include <string.h>

static bool
atom_equ(const atom_t a, const atom_t b)
{
  switch (a->type) {
    case T_NIL:
    case T_TRUE:
    case T_WILDCARD:
      return true;
    case T_NUMBER:
      return a->number == b->number;
    case T_PAIR:
      return lisp_equ(a->pair.car, b->pair.car) &&
        lisp_equ(a->pair.car, b->pair.car);
    case T_STRING:
      return strcmp(a->string, b->string) == 0;
    case T_SYMBOL:
      return lisp_symbol_match(a, b);
  }
}

bool
lisp_equ(const atom_t a, const atom_t b)
{
  return a->type == b->type && atom_equ(a, b);
}
