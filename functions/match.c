#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <string.h>

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
      return lisp_symbol_match(a, b);
    default:
      return true;
  }
}

static atom_t
lisp_match(const atom_t closure, const atom_t cell, const atom_t match)
{
  /*
   * Sanity checks.
   */
  if (IS_NULL(match) || !IS_PAIR(match) || !IS_PAIR(CAR(match))) {
    X(cell); X(match);
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
    X(args); X(cdr); X(cell);
    return lisp_eval(closure, prog);
  }
  /*
   */
  X(args); X(prog);
  return lisp_match(closure, cell, cdr);
}

atom_t
lisp_function_match(const atom_t closure, const atom_t cell)
{
  atom_t car = lisp_eval(closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  return lisp_match(closure, car, cdr);
}

LISP_REGISTER(match, match)