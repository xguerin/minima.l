#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_setq(const lisp_t lisp, const atom_t closure)
{
  atom_t val;
  LISP_ARGS(closure, C, ANY);
  /*
   * Extract the symbol.
   */
  atom_t symb = lisp_car(lisp, ANY);
  if (!IS_SCOP(symb) && !IS_SYMB(symb)) {
    X(lisp->slab, symb);
    return lisp_make_nil(lisp);
  }
  /*
   * Extract the value.
   */
  atom_t cdr = lisp_cdr(lisp, ANY);
  atom_t res = lisp_eval(lisp, C, lisp_car(lisp, cdr));
  X(lisp->slab, cdr);
  /*
   * Don't set anything if NIL.
   */
  if (unlikely(IS_NULL(res))) {
    X(lisp->slab, symb);
    return res;
  }
  /*
   * If the symbol is scoped, update the value and prepare the scope.
   */
  if (IS_SCOP(symb)) {
    /*
     * Grab the scope's name and symbol.
     */
    atom_t nsp = lisp_scope_get_name(lisp, symb);
    atom_t sym = lisp_scope_get_symb(lisp, symb);
    X(lisp->slab, symb);
    /*
     * Grab the scope and update the value.
     */
    atom_t scp = lisp_lookup(lisp, lisp->globals, closure, &nsp->symbol);
    LISP_SETQ(lisp, scp, lisp_cons(lisp, sym, UP(res)));
    /*
     * Prepare the scope.
     */
    val = lisp_cons(lisp, nsp, scp);
  }
  /*
   * Otherwise, just rrepare the value.
   */
  else {
    val = lisp_cons(lisp, symb, UP(res));
  }
  /*
   * Update the globals with the new scope or value.
   */
  LISP_SETQ(lisp, lisp->globals, val);
  return res;
}

LISP_MODULE_SETUP(setq, setq, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
