#include "mnml/debug.h"
#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t USED
lisp_function_def(const lisp_t lisp, const atom_t closure)
{
  atom_t val;
  LISP_ARGS(closure, C, ANY);
  /*
   * Grab the symbol.
   */
  atom_t symb = lisp_car(lisp, ANY);
  if (!IS_SCOP(symb) && !IS_SYMB(symb)) {
    X(lisp->slab, symb);
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
   * Don't set anything if NIL.
   */
  if (unlikely(IS_NULL(prog))) {
    X(lisp->slab, symb, args, prog);
    return lisp_make_nil(lisp);
  }
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
   * If the symbol is scoped, update the value and prepare the scope.
   */
  if (IS_SCOP(symb)) {
    /*
     * Grab the scope's name and symbol.
     */
    atom_t nsp = lisp_scope_get_name(lisp, symb);
    atom_t sym = lisp_scope_get_symb(lisp, symb);
    /*
     * Grab the scope and update the value.
     */
    atom_t scp = lisp_lookup(lisp, lisp->globals, closure, &nsp->symbol);
    LISP_SETQ(lisp, scp, lisp_cons(lisp, sym, con1));
    /*
     * Prepare the scope.
     */
    val = lisp_cons(lisp, nsp, scp);

  }
  /*
   * Otherwise, just rrepare the value.
   */
  else {
    val = lisp_cons(lisp, UP(symb), con1);
  }
  /*
   * Update the globals with the new scope or value.
   */
  LISP_SETQ(lisp, lisp->globals, val);
  return symb;
}

LISP_MODULE_SETUP(def, def, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
