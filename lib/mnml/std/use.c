#include "mnml/types.h"
#include <mnml/debug.h>
#include <mnml/maker.h>
#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <fcntl.h>
#include <limits.h>

static atom_t
get_symbols(const lisp_t lisp, const atom_t cell)
{
  /*
   * Default case.
   */
  if (IS_NULL(cell)) {
    return lisp_make_nil(lisp);
  }
  /*
   * Recursive descent.
   */
  atom_t sym = lisp_car(lisp, CAR(cell));
  return lisp_cons(lisp, sym, get_symbols(lisp, CDR(cell)));
}

static atom_t USED
lisp_use(const lisp_t lisp, const atom_t closure, const atom_t cell)
{
  atom_t res;
  /*
   * Get CAR/CDR.
   */
  atom_t car = lisp_eval(lisp, closure, lisp_car(lisp, cell));
  atom_t cdr = lisp_cdr(lisp, cell);
  X(lisp->slab, cell);
  /*
   * Make sure that we are dealing with a symbol or a list.
   */
  switch (car->type) {
    /*
     * If it's a scoped symbol, use the one symbol from the scope
     * (shortcut for '(MOD SYM)).
     */
    case T_SCOPED_SYMBOL: {
      /*
       * Get the namespace and the symbol.
       */
      atom_t nsp = lisp_scope_get_name(lisp, car);
      atom_t sym = lisp_scope_get_symb(lisp, car);
      X(lisp->slab, car);
      /*
       * Import the symbol.
       */
      res = lisp_import(lisp, closure, nsp,
                        lisp_cons(lisp, sym, lisp_make_nil(lisp)),
                        lisp_make_nil(lisp));
      X(lisp->slab, nsp);
      break;
    }
    /*
     * If it's a symbol, load all the scope.
     */
    case T_SYMBOL: {
      /*
       * Lookup the namespace.
       */
      atom_t nil = lisp_make_nil(lisp);
      atom_t scp = lisp_lookup(lisp, lisp->scopes, nil, &car->symbol);
      X(lisp->slab, nil);
      /*
       * Import all of its content.
       */
      atom_t symbols = get_symbols(lisp, scp);
      TRACE_EVAL_SEXP(symbols);
      res = lisp_import(lisp, closure, car, symbols, lisp_make_nil(lisp));
      X(lisp->slab, scp, car);
      break;
    }
    /*
     * If it's a pair, it's a list of symbols.
     */
    case T_PAIR: {
      if (!IS_SYMB(CAR(car)) && !IS_PAIR(CDR(car))) {
        X(lisp->slab, car);
        res = lisp_make_nil(lisp);
      } else {
        atom_t scp = lisp_car(lisp, car);
        atom_t lst = lisp_cdr(lisp, car);
        X(lisp->slab, car);
        res = lisp_import(lisp, closure, scp, lst, lisp_make_nil(lisp));
        X(lisp->slab, scp);
      }
      break;
    }
    /*
     * Other forms are not supported.
     */
    default: {
      X(lisp->slab, car);
      res = lisp_make_nil(lisp);
      break;
    }
  }
  /*
   * If CDR is NIL, return the result.
   */
  if (IS_NULL(cdr)) {
    X(lisp->slab, cdr);
    return res;
  }
  /*
   * Return the next evaluation otherwise.
   */
  X(lisp->slab, res);
  return lisp_use(lisp, closure, cdr);
}

static atom_t
lisp_function_use(const lisp_t lisp, const atom_t closure)
{
  TRACE_CLOS_SEXP(closure);
  LISP_ARGS(closure, C, ANY);
  return lisp_use(lisp, C, UP(ANY));
}

LISP_MODULE_SETUP(use, use, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
