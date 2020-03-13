#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * List context functions.
 */

lisp_t
lisp_new(const atom_t ichan, const atom_t ochan)
{
  lisp_t lisp = (lisp_t)malloc(sizeof(struct _lisp));
  lisp->globals = UP(NIL);
  lisp->ichan = UP(ichan);
  lisp->ochan = UP(ochan);
  return lisp;
}

void
lisp_delete(lisp_t lisp)
{
  X(lisp->ochan);
  X(lisp->ichan);
  X(lisp->globals);
  free(lisp);
}

/*
 * Global symbols.
 */

atom_t NIL = NULL;
atom_t TRUE = NULL;
atom_t QUOTE = NULL;
atom_t WILDCARD = NULL;

/*
 * Symbol lookup.
 */

atom_t
lisp_lookup(const lisp_t lisp, const atom_t closure, const symbol_t sym)
{
  /*
   * Look for the symbol the closure.
   */
  FOREACH(closure, a)
  {
    atom_t car = a->car;
    if (lisp_symbol_match(CAR(car), sym)) {
      return UP(CDR(car));
    }
    NEXT(a);
  }
  /*
   * Check the global environment.
   */
  FOREACH(GLOBALS, g)
  {
    atom_t car = g->car;
    if (lisp_symbol_match(CAR(car), sym)) {
      return UP(CDR(car));
    }
    NEXT(g);
  }
  /*
   * Nothing found.
   */
  return UP(NIL);
}

/*
 * Basic functions.
 */

atom_t
lisp_car(const atom_t cell)
{
  if (likely(IS_PAIR(cell))) {
    return UP(CAR(cell));
  }
  return UP(NIL);
}

atom_t
lisp_cdr(const atom_t cell)
{
  if (likely(IS_PAIR(cell))) {
    return UP(CDR(cell));
  }
  return UP(NIL);
}

/*
 * Internal list construction functions.
 */

atom_t
lisp_cons(const atom_t car, const atom_t cdr)
{
  atom_t R = lisp_allocate();
  R->type = T_PAIR;
  R->refs = 1;
  CAR(R) = UP(car);
  CDR(R) = UP(cdr);
  TRACE_CONS_SEXP(R);
  return R;
}

atom_t
lisp_conc(const atom_t car, const atom_t cdr)
{
  atom_t R;
  /*
   */
  if (likely(IS_PAIR(car))) {
    FOREACH(car, p) {
      NEXT(p);
    }
    X(p->cdr);
    p->cdr = UP(cdr);
    R = UP(car);
  } else {
    R = UP(cdr);
  }
  /*
   */
  TRACE_CONS_SEXP(R);
  return R;
}

/*
 * SETQ. PAIR is consumed.
 */

atom_t
lisp_setq(const atom_t closure, const atom_t pair)
{
  /*
   * Check if pair is valid.
   */
  if (!IS_PAIR(pair) || !IS_SYMB(CAR(pair))) {
    X(pair);
    return UP(closure);
  }
  /*
   * If the closure is NIL, return the wrapped pair.
   */
  if (IS_NULL(closure)) {
    atom_t res = lisp_cons(pair, closure);
    X(pair);
    return res;
  }
  /*
   * Extract CAR and CDR.
   */
  atom_t car = lisp_car(closure);
  atom_t cdr = lisp_cdr(closure);
  /*
   * Replace car if its symbol matches pair's.
   */
  if (lisp_symbol_match(CAR(car), &CAR(pair)->symbol)) {
    atom_t res = lisp_cons(pair, cdr);
    X(pair, cdr, car);
    return res;
  }
  /*
   * Look further down the closure.
   */
  atom_t nxt = lisp_setq(cdr, pair);
  atom_t res = lisp_cons(car, nxt);
  X(cdr, car, nxt);
  return res;
}

/*
 * PROG. CELL and RESULT are consumed.
 */

atom_t
lisp_prog(const lisp_t lisp, const atom_t closure, const atom_t cell,
          const atom_t result)
{
  if (likely(IS_PAIR(cell))) {
    /*
     * Get CAR/CDR.
     */
    atom_t res = lisp_eval(lisp, closure, lisp_car(cell));
    atom_t cdr = lisp_cdr(cell);
    /*
     */
    X(cell, result);
    return lisp_prog(lisp, closure, cdr, res);
  }
  /*
   */
  X(cell);
  return result;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
