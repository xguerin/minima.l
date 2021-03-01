#pragma once

#include <mnml/slab.h>
#include <mnml/types.h>
#include <stdbool.h>

/*
 * Helper macros.
 */

#define FOREACH(__c, __p)    \
  pair_t __p = &(__c)->pair; \
  if (!IS_NULL(__c))         \
    for (;;)

#define NEXT(__p)           \
  if (!IS_PAIR((__p)->cdr)) \
    break;                  \
  (__p) = &(__p)->cdr->pair

/*
 * Lisp context type.
 */

typedef struct lisp
{
  slab_t slab;
  atom_t globals;
  atom_t modules;
  atom_t ichan;
  atom_t ochan;
} * lisp_t;

lisp_t lisp_new(const slab_t slab);
void lisp_delete(const lisp_t lisp);

/*
 * Allocation functions.
 */

atom_t lisp_allocate(const lisp_t lisp);
void lisp_deallocate(const lisp_t lisp, const atom_t cell);

/*
 * X macro.
 */

#define X_0(_l, __a)                  \
  do {                                \
    DOWN(__a);                        \
    if (unlikely((__a)->refs == 0)) { \
      lisp_deallocate(_l, __a);       \
    }                                 \
  } while (0)

#define X_1(_l, _1) \
  do {              \
    X_0(_l, _1);    \
  } while (0)

#define X_2(_l, _2, ...)  \
  do {                    \
    X_0(_l, _2);          \
    X_1(_l, __VA_ARGS__); \
  } while (0)

#define X_3(_l, _3, ...)  \
  do {                    \
    X_0(_l, _3);          \
    X_2(_l, __VA_ARGS__); \
  } while (0)

#define X_4(_l, _4, ...)  \
  do {                    \
    X_0(_l, _4);          \
    X_3(_l, __VA_ARGS__); \
  } while (0)

#define X_5(_l, _5, ...)  \
  do {                    \
    X_0(_l, _5);          \
    X_4(_l, __VA_ARGS__); \
  } while (0)

#define X_6(_l, _6, ...)  \
  do {                    \
    X_0(_l, _6);          \
    X_5(_l, __VA_ARGS__); \
  } while (0)

#define X_7(_l, _7, ...)  \
  do {                    \
    X_0(_l, _7);          \
    X_6(_l, __VA_ARGS__); \
  } while (0)

#define X_8(_l, _8, ...)  \
  do {                    \
    X_0(_l, _8);          \
    X_7(_l, __VA_ARGS__); \
  } while (0)

#define X_(_1, _2, _3, _4, _5, _6, _7, _8, NAME, ...) NAME
#define X(_l, ...) \
  X_(__VA_ARGS__, X_8, X_7, X_6, X_5, X_4, X_3, X_2, X_1)(_l, __VA_ARGS__)

/*
 * Native function type.
 */

typedef atom_t (*function_t)(const lisp_t, const atom_t);

/*
 * Symbol lookup.
 */

atom_t lisp_lookup(const lisp_t lisp, const atom_t closure, const symbol_t sym);

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

/*
 * Read, eval, print functions.
 */

atom_t lisp_read(const lisp_t lisp, const atom_t cell);
atom_t lisp_eval(const lisp_t lisp, const atom_t closure, const atom_t cell);
void lisp_prin(const lisp_t lisp, const atom_t cell, const bool s);

// vim: tw=80:sw=2:ts=2:sts=2:et
