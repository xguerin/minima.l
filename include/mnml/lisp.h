#pragma once

#include <mnml/types.h>
#include <stdbool.h>

/*
 * Helper macros.
 */

#define FOREACH(__c, __p)     \
  pair_t __p = &__c->pair;    \
  if (!IS_NULL(__c)) for (;;)

#define NEXT(__p) {               \
  if (!IS_PAIR(__p->cdr)) break;  \
  __p = &__p->cdr->pair;          \
}

/*
 * Symbol management.
 */

extern atom_t ICHAN;
extern atom_t OCHAN;
extern atom_t NIL;
extern atom_t TRUE;
extern atom_t QUOTE;
extern atom_t WILDCARD;

atom_t lisp_lookup_immediate(const atom_t closure, const symbol_t sym);
atom_t lisp_lookup(const lisp_t lisp, const atom_t closure, const atom_t sym);

/*
 * Lisp basic functions.
 */

atom_t lisp_car(const atom_t cell);
atom_t lisp_cdr(const atom_t cell);

/*
 * Internal list construction functions. CONS is pure, CONC is destructive.
 */

atom_t lisp_cons(const atom_t a, const atom_t b);
atom_t lisp_conc(const atom_t a, const atom_t b);

/*
 * Evaluation and closure functions.
 */

atom_t lisp_bind(const lisp_t lisp, const atom_t closure, const atom_t args,
                 const atom_t vals);
atom_t lisp_setq(const atom_t closure, const atom_t pair);
atom_t lisp_prog(const lisp_t lisp, const atom_t closure, const atom_t cell,
                 const atom_t rslt);

/*
 * Read, eval, print functions.
 */

atom_t lisp_read(const lisp_t lisp, const atom_t closure, const atom_t cell);
atom_t lisp_eval(const lisp_t lisp, const atom_t closure, const atom_t cell);
void   lisp_prin(const atom_t closure, const atom_t cell, const bool s);

// vim: tw=80:sw=2:ts=2:sts=2:et
