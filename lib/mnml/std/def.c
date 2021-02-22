#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t USED
lisp_function_def(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, ANY);
  /*
   * Grab the symbol.
   */
  atom_t symb = lisp_car(lisp, ANY);
  if (!IS_SYMB(symb)) {
    return lisp_make_nil(lisp);
  }
  /*
   * Grab the arguments body.
   */
  atom_t cdr0 = lisp_cdr(lisp, ANY);
  atom_t args = lisp_car(lisp, cdr0);
  if (!(IS_NULL(args) || IS_PAIR(args) || IS_SYMB(args))) {
    X(lisp->slab, symb, cdr0, args);
    return lisp_make_nil(lisp);
  }
  /*
   * Grab the body.
   */
  atom_t prog = lisp_cdr(lisp, cdr0);
  X(lisp->slab, cdr0);
  /*
   * Check if there is a docstring in the declaration.
   */
  atom_t doc = lisp_car(lisp, prog);
  if (IS_PAIR(doc) && IS_CHAR(CAR(doc))) {
    atom_t nxt = lisp_cdr(lisp, prog);
    X(lisp->slab, prog);
    prog = nxt;
  }
  X(lisp->slab, doc);
  /*
   * Check if there is any tail call.
   */
  lisp_mark_tail_calls(lisp, symb, args, prog);
  /*
   * Append an empty closure.
   */
  atom_t con0 = lisp_cons(lisp, lisp_make_nil(lisp), prog);
  atom_t con1 = lisp_cons(lisp, args, con0);
  /*
   * Set the symbol's value.
   */
  atom_t tmp = GLOBALS;
  GLOBALS = lisp_setq(lisp, GLOBALS, lisp_cons(lisp, UP(symb), con1));
  X(lisp->slab, tmp);
  return symb;
}

LISP_MODULE_SETUP(def, def, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
