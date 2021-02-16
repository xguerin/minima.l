#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static bool
atom_match(const atom_t a, const atom_t b)
{
  if (IS_WILD(a)) {
    return true;
  }
  if (a->type != b->type) {
    return false;
  }
  switch (a->type) {
    case T_CHAR:
    case T_NUMBER:
      return a->number == b->number;
    case T_PAIR:
      return atom_match(CAR(a), CAR(b)) && atom_match(CDR(a), CDR(b));
    case T_SYMBOL:
      return lisp_symbol_match(a, &b->symbol);
    default:
      return true;
  }
}

static atom_t
lisp_match(const lisp_t lisp, const atom_t closure, const atom_t cell,
           const atom_t match)
{
  /*
   * Sanity checks.
   */
  if (IS_NULL(match) || !IS_PAIR(match) || !IS_PAIR(CAR(match))) {
    X(cell, match);
    return UP(NIL);
  }
  /*
   * Get CAR/CDR.
   */
  atom_t car = lisp_car(match);
  atom_t cdr = lisp_cdr(match);
  X(match);
  /*
   * Get args and prog.
   */
  atom_t args = lisp_car(car);
  atom_t prog = lisp_cdr(car);
  X(car);
  /*
   * Match the cell with CAR.
   */
  if (atom_match(args, cell)) {
    X(args, cdr, cell);
    return lisp_eval(lisp, closure, prog);
  }
  /*
   */
  X(args, prog);
  return lisp_match(lisp, closure, cell, cdr);
}

static atom_t USED
lisp_function_match(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, cell, closure, @);
  atom_t car = lisp_eval(lisp, closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  atom_t res = lisp_match(lisp, closure, car, cdr);
  X(cell);
  return res;
}

LISP_MODULE_SETUP(match, match, @)

// vim: tw=80:sw=2:ts=2:sts=2:et