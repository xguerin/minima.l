#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_function_def(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, cell, closure, @);
  /*
   * Grab the symbol.
   */
  atom_t symb = lisp_car(cell);
  if (!IS_SYMB(symb)) {
    X(symb, cell);
    return UP(NIL);
  }
  /*
   * Grab the arguments body.
   */
  atom_t cdr0 = lisp_cdr(cell);
  atom_t args = lisp_car(cdr0);
  if (!(IS_NULL(args) || IS_PAIR(args) || IS_SYMB(args))) {
    X(symb, cell, cdr0, args);
    return UP(NIL);
  }
  /*
   * Grab the body.
   */
  atom_t prog = lisp_cdr(cdr0);
  X(cell, cdr0);
  /*
   * Check if there is a docstring in the declaration.
   */
  atom_t doc = lisp_car(prog);
  if (IS_PAIR(doc) && IS_CHAR(CAR(doc))) {
    atom_t nxt = lisp_cdr(prog);
    X(prog);
    prog = nxt;
  }
  X(doc);
  /*
   * Append an empty closure.
   */
  atom_t con0 = lisp_cons(NIL, prog);
  atom_t con1 = lisp_cons(args, con0);
  X(prog, args, con0);
  /*
   * Set the symbol's value.
   */
  atom_t tmp = lisp->GLOBALS;
  lisp->GLOBALS = lisp_setq(lisp->GLOBALS, lisp_cons(symb, con1));
  X(tmp, con1);
  return symb;
}

LISP_MODULE_SETUP(def, def, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
