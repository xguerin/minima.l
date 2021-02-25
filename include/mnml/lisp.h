#pragma once

#include <mnml/slab.h>
#include <mnml/types.h>
#include <stdbool.h>

/*
 * Helper macros.
 */

#define FOREACH(__c, __p)  \
  pair_t __p = &__c->pair; \
  if (!IS_NULL(__c))       \
    for (;;)

#define NEXT(__p)         \
  if (!IS_PAIR(__p->cdr)) \
    break;                \
  __p = &__p->cdr->pair

/*
 * Lisp context type.
 */

typedef struct _lisp
{
  slab_t slab;
  atom_t globals;
  atom_t scopes;
  atom_t modules;
  atom_t ichan;
  atom_t ochan;
} * lisp_t;

lisp_t lisp_new(const slab_t slab);
void lisp_delete(const lisp_t lisp);

/*
 * Native function type.
 */

typedef atom_t (*function_t)(const lisp_t, const atom_t);

/*
 * Symbol lookup.
 */

atom_t lisp_lookup(const lisp_t lisp, const atom_t scope, const atom_t closure,
                   const symbol_t sym);

/*
 * Lisp basic functions.
 */

atom_t lisp_car(const lisp_t lisp, const atom_t cell);
atom_t lisp_cdr(const lisp_t lisp, const atom_t cell);

/*
 * Internal list construction functions. CONS is pure, CONC is destructive.
 * NOTE Both functions consume their arguments.
 */

atom_t lisp_cons(const lisp_t lisp, const atom_t a, const atom_t b);
atom_t lisp_conc(const lisp_t lisp, const atom_t a, const atom_t b);

/*
 * Evaluation and closure functions.
 */

atom_t lisp_bind(const lisp_t lisp, const atom_t closure, const atom_t args,
                 const atom_t vals);
atom_t lisp_setq(const lisp_t lisp, const atom_t closure, const atom_t pair);
atom_t lisp_prog(const lisp_t lisp, const atom_t closure, const atom_t cell,
                 const atom_t rslt);

#define LISP_SETQ(__l, __c, __v)    \
  {                                 \
    atom_t tmp = __c;               \
    __c = lisp_setq(__l, tmp, __v); \
    X(__l->slab, tmp);              \
  }

/*
 * Read, eval, print functions.
 */

atom_t lisp_read(const lisp_t lisp, const atom_t closure, const atom_t cell);
atom_t lisp_eval(const lisp_t lisp, const atom_t closure, const atom_t cell);
void lisp_prin(const lisp_t lisp, const atom_t closure, const atom_t cell,
               const bool s);

// vim: tw=80:sw=2:ts=2:sts=2:et
