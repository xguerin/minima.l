#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

static atom_t
lisp_function_def(const atom_t closure, const atom_t cell)
{
  /*
   * Grab the symbol, the arguments, and the body.
   */
  atom_t symb = lisp_car(cell);
  atom_t cdr0 = lisp_cdr(cell);
  X(cell);
  atom_t args = lisp_car(cdr0);
  atom_t prog = lisp_cdr(cdr0);
  X(cdr0);
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
   * Append a dummy closure.
   */
  atom_t con0 = lisp_cons(NIL, prog);
  X(prog);
  atom_t con1 = lisp_cons(args, con0);
  X(args); X(con0);
  /*
   * Set the symbol's value.
   */
  GLOBALS = lisp_setq(GLOBALS, lisp_cons(symb, con1));
  X(symb);
  return con1;
}

LISP_REGISTER(def, def)
