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
lisp_match(const lisp_t lisp, const atom_t closure, const atom_t ANY,
           const atom_t match)
{
  /*
   * Sanity checks.
   */
  if (IS_NULL(match) || !IS_PAIR(match) || !IS_PAIR(CAR(match))) {
    X(ANY, match);
    return lisp_make_nil();
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
   * Match the ANY with CAR.
   */
  if (atom_match(args, ANY)) {
    X(args, cdr, ANY);
    return lisp_eval(lisp, closure, prog);
  }
  /*
   */
  X(args, prog);
  return lisp_match(lisp, closure, ANY, cdr);
}

static atom_t USED
lisp_function_match(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, ANY);
  atom_t car = lisp_eval(lisp, closure, lisp_car(ANY));
  atom_t cdr = lisp_cdr(ANY);
  return lisp_match(lisp, C, car, cdr);
}

LISP_MODULE_SETUP(match, match, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
