#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_function_def(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(cell, arguments, @);
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
  if (!IS_NULL(args) && !IS_PAIR(args)) {
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
   * Append an empty currying list and closure.
   */
  atom_t con0 = lisp_cons(NIL, prog);
  atom_t con1 = lisp_cons(NIL, con0);
  atom_t con2 = lisp_cons(args, con1);
  X(prog, args, con0, con1);
  /*
   * Set the symbol's value.
   */
  GLOBALS = lisp_setq(GLOBALS, lisp_cons(symb, con2));
  X(symb);
  return con2;
}

LISP_PLUGIN_REGISTER(def, def, @)
