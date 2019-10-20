#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static bool lisp_neq(const atom_t a, const atom_t b);

static bool
atom_neq(const atom_t a, const atom_t b)
{
  switch (a->type) {
    case T_NIL:
    case T_TRUE:
    case T_WILDCARD:
      return false;
    case T_CHAR:
    case T_NUMBER:
      return a->number != b->number;
    case T_PAIR:
      return lisp_neq(CAR(a), CAR(b)) || lisp_neq(CDR(a), CDR(b));
    case T_SYMBOL:
      return !lisp_symbol_match(a, b);
    default:
      return false;
  }
}

static bool
lisp_neq(const atom_t a, const atom_t b)
{
  return a->type != b->type || atom_neq(a, b);
}

static atom_t
lisp_function_neq(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(vl0, arguments, X);
  LISP_LOOKUP(vl1, arguments, Y);
  atom_t res = lisp_neq(vl0, vl1) ? TRUE : NIL;
  X(vl0); X(vl1);
  return UP(res);
}

LISP_PLUGIN_REGISTER(neq, <>, X, Y, NIL)
