#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static bool lisp_equ(const atom_t a, const atom_t b);

static bool
atom_equ(const atom_t a, const atom_t b)
{
  switch (a->type) {
    case T_NIL:
    case T_TRUE:
    case T_WILDCARD:
      return true;
    case T_CHAR:
    case T_NUMBER:
      return a->number == b->number;
    case T_PAIR:
      return lisp_equ(CAR(a), CAR(b)) && lisp_equ(CDR(a), CDR(b));
    case T_SYMBOL:
      return lisp_symbol_match(a, b);
    default:
      return false;
  }
}

static bool
lisp_equ(const atom_t a, const atom_t b)
{
  return a->type == b->type && atom_equ(a, b);
}

static atom_t
lisp_function_equ(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(vl0, arguments, X);
  LISP_LOOKUP(vl1, arguments, Y);
  atom_t res = lisp_equ(vl0, vl1) ? TRUE : NIL;
  X(vl0); X(vl1);
  return UP(res);
}

LISP_PLUGIN_REGISTER(equ, =, X, Y, NIL)