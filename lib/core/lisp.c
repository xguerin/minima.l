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
lisp_new(const slab_t slab)
{
  lisp_t lisp = (lisp_t)malloc(sizeof(struct lisp));
  lisp->slab = slab;
  lisp->globals = lisp_make_nil(lisp);
  lisp->ichan = lisp_make_nil(lisp);
  lisp->ochan = lisp_make_nil(lisp);
  lisp->lrefs = 0;
  lisp->crefs = 0;
  lisp->grefs = 0;
  lisp->total = 0;
  return lisp;
}

void
lisp_delete(lisp_t lisp)
{
  TRACE("R %ld %ld %ld %ld", lisp->lrefs, lisp->crefs, lisp->grefs,
        lisp->total);
  X(lisp, lisp->ochan);
  X(lisp, lisp->ichan);
  X(lisp, lisp->globals);
  free(lisp);
}

/*
 * Allocation functions.
 */

atom_t
lisp_allocate(const lisp_t lisp)
{
  return slab_allocate(lisp->slab);
}

void
lisp_deallocate(const lisp_t lisp, const atom_t atom)
{
  TRACE_SLAB_SEXP(atom);
  /*
   * Most likely this is a pair.
   */
  if (likely(IS_PAIR(atom))) {
    X(lisp, CAR(atom));
    X(lisp, CDR(atom));
    slab_deallocate(lisp->slab, atom);
  }
  /*
   * Process atoms.
   */
  else {
    slab_deallocate(lisp->slab, atom);
  }
}

/*
 * Symbol lookup.
 */

atom_t
lisp_lookup(const lisp_t lisp, const atom_t closure, const atom_t atom)
{
  lisp->total += 1;
  /*
   * Look for the symbol the closure.
   */
  FOREACH(closure, a)
  {
    atom_t car = a->car;
    if (lisp_symbol_match(CAR(car), &atom->symbol)) {
      lisp->lrefs += 1;
      return UP(CDR(car));
    }
    NEXT(a);
  }
  /*
   * Check if there is any cached value.
   */
  if (atom->cache != NULL) {
    lisp->crefs += 1;
    return UP(CDR(atom->cache));
  }
  /*
   * Check the global environment.
   */
  FOREACH(lisp->globals, g)
  {
    atom_t car = g->car;
    if (lisp_symbol_match(CAR(car), &atom->symbol)) {
      lisp->grefs += 1;
      atom->cache = car;
      return UP(CDR(car));
    }
    NEXT(g);
  }
  /*
   * Nothing found.
   */
  return lisp_make_nil(lisp);
}

/*
 * Basic functions.
 */

atom_t
lisp_car(const lisp_t lisp, const atom_t cell)
{
  if (likely(IS_PAIR(cell))) {
    return UP(CAR(cell));
  }
  return lisp_make_nil(lisp);
}

atom_t
lisp_cdr(const lisp_t lisp, const atom_t cell)
{
  if (likely(IS_PAIR(cell))) {
    return UP(CDR(cell));
  }
  return lisp_make_nil(lisp);
}

/*
 * Internal list construction functions.
 */

atom_t
lisp_cons(const lisp_t lisp, const atom_t car, const atom_t cdr)
{
  atom_t R = lisp_allocate(lisp);
  R->type = T_PAIR;
  R->refs = 1;
  CAR(R) = car;
  CDR(R) = cdr;
  TRACE_CONS_SEXP(R);
  return R;
}

atom_t
lisp_conc(const lisp_t lisp, const atom_t car, const atom_t cdr)
{
  atom_t R;
  /*
   */
  if (likely(IS_PAIR(car))) {
    FOREACH(car, p) { NEXT(p); }
    X(lisp, p->cdr);
    p->cdr = cdr;
    R = car;
  } else {
    X(lisp, car);
    R = cdr;
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
lisp_setq(const lisp_t lisp, const atom_t closure, const atom_t pair)
{
  /*
   * Check if pair is valid.
   */
  if (!IS_PAIR(pair) || !IS_SYMB(CAR(pair))) {
    X(lisp, pair);
    return UP(closure);
  }
  /*
   * If the closure is NIL, return the wrapped pair.
   */
  if (IS_NULL(closure)) {
    return lisp_cons(lisp, pair, UP(closure));
  }
  /*
   * Scan the closure for an existing key.
   */
  FOREACH(closure, g)
  {
    atom_t car = g->car;
    if (lisp_symbol_match(CAR(car), &CAR(pair)->symbol)) {
      const atom_t old = CDR(car);
      CDR(car) = CDR(pair);
      CDR(pair) = old;
      X(lisp, pair);
      return UP(closure);
    }
    NEXT(g);
  }
  /*
   * If not, just append the new pair.
   */
  return lisp_cons(lisp, pair, UP(closure));
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
    atom_t res = lisp_eval(lisp, closure, lisp_car(lisp, cell));
    atom_t cdr = lisp_cdr(lisp, cell);
    /*
     */
    X(lisp, cell, result);
    return lisp_prog(lisp, closure, cdr, res);
  }
  /*
   */
  X(lisp, cell);
  return result;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
