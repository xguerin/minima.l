#pragma once

#include <mnml/compiler.h>
#include <mnml/debug.h>
#include <mnml/slab.h>
#include <mnml/types.h>
#include <stdbool.h>
#include <string.h>

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
  size_t lrefs;
  size_t crefs;
  size_t grefs;
  size_t total;
}* lisp_t;

/*
 * Native function type.
 */

typedef atom_t (*function_t)(const lisp_t, const atom_t);

/*
 * Context allocation.
 */

lisp_t lisp_new(const slab_t slab);
void lisp_delete(const lisp_t lisp);

/*
 * Atom allocation.
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
 * Symbol makers.
 */

#define MAKE_SYMBOL_STATIC(__v, __s)                          \
  symbol_t __v = (union symbol*)alloca(sizeof(union symbol)); \
  (__v)->tag = NULL_TAG;                                      \
  strncpy((__v)->val, __s, LISP_SYMBOL_LENGTH)

#define MAKE_SYMBOL_STATIC_N(__v, __s, __n)                              \
  size_t __l = (__n) <= LISP_SYMBOL_LENGTH ? (__n) : LISP_SYMBOL_LENGTH; \
  symbol_t __v = (union symbol*)alloca(sizeof(union symbol));            \
  (__v)->tag = NULL_TAG;                                                 \
  strncpy((__v)->val, __s, __l)

#define MAKE_SYMBOL_DYNAMIC(__v, __s)                         \
  symbol_t __v = (union symbol*)malloc(sizeof(union symbol)); \
  (__v)->tag = NULL_TAG;                                      \
  strncpy((__v)->val, __s, LISP_SYMBOL_LENGTH)

#define MAKE_SYMBOL_DYNAMIC_N(__v, __s, __n)                             \
  size_t __l = (__n) <= LISP_SYMBOL_LENGTH ? (__n) : LISP_SYMBOL_LENGTH; \
  symbol_t __v = (union symbol*)malloc(sizeof(union symbol));            \
  (__v)->tag = NULL_TAG;                                                 \
  strncpy((__v)->val, __s, __l)

/*
 * Atom makers.
 */

ALWAYS_INLINE inline atom_t
lisp_make_char(const lisp_t lisp, const char c)
{
  atom_t R = lisp_allocate(lisp);
  R->type = T_CHAR;
  R->flags = 0;
  R->refs = 1;
  R->number = (int64_t)c;
  TRACE_MAKE_SEXP(R);
  return R;
}

ALWAYS_INLINE inline atom_t
lisp_make_number(const lisp_t lisp, const int64_t num)
{
  atom_t R = lisp_allocate(lisp);
  R->type = T_NUMBER;
  R->flags = 0;
  R->refs = 1;
  R->number = num;
  TRACE_MAKE_SEXP(R);
  return R;
}

ALWAYS_INLINE inline atom_t
lisp_make_nil(const lisp_t lisp)
{
  atom_t R = lisp_allocate(lisp);
  R->type = T_NIL;
  R->flags = 0;
  R->refs = 1;
  TRACE_MAKE_SEXP(R);
  return R;
}

ALWAYS_INLINE inline atom_t
lisp_make_true(const lisp_t lisp)
{
  atom_t R = lisp_allocate(lisp);
  R->type = T_TRUE;
  R->flags = 0;
  R->refs = 1;
  TRACE_MAKE_SEXP(R);
  return R;
}

ALWAYS_INLINE inline atom_t
lisp_make_symbol(const lisp_t lisp, const symbol_t sym)
{
  atom_t R = lisp_allocate(lisp);
  R->type = T_SYMBOL;
  R->flags = 0;
  R->refs = 1;
  R->symbol = *sym;
  TRACE_MAKE_SEXP(R);
  return R;
}

ALWAYS_INLINE inline atom_t
lisp_make_wildcard(const lisp_t lisp)
{
  atom_t R = lisp_allocate(lisp);
  R->type = T_WILDCARD;
  R->flags = 0;
  R->refs = 1;
  TRACE_MAKE_SEXP(R);
  return R;
}

ALWAYS_INLINE inline atom_t
lisp_make_quote(const lisp_t lisp)
{
  MAKE_SYMBOL_STATIC(quote, "quote");
  atom_t R = lisp_make_symbol(lisp, quote);
  TRACE_MAKE_SEXP(R);
  return R;
}

atom_t lisp_make_string(const lisp_t lisp, const char* const str,
                        const size_t len);

/*
 * FFI-specific interface.
 */

void lisp_io_push(const lisp_t lisp);
void lisp_io_pop(const lisp_t lisp);

ALWAYS_INLINE inline atom_t
lisp_make_symbol_from_string(const lisp_t lisp, const char* const str,
                             const size_t len)
{
  MAKE_SYMBOL_STATIC_N(sym, str, len);
  atom_t R = lisp_allocate(lisp);
  R->type = T_SYMBOL;
  R->flags = 0;
  R->refs = 1;
  R->symbol = *sym;
  TRACE_MAKE_SEXP(R);
  return R;
}

ALWAYS_INLINE inline int
lisp_get_type(const atom_t atom)
{
  return atom->type;
}

ALWAYS_INLINE inline char
lisp_get_char(const atom_t atom)
{
  return (char)atom->number;
}

ALWAYS_INLINE inline int64_t
lisp_get_number(const atom_t atom)
{
  return atom->number;
}

ALWAYS_INLINE inline const char*
lisp_get_symbol(const atom_t atom)
{
  return atom->symbol.val;
}

ALWAYS_INLINE inline void
lisp_drop(const lisp_t lisp, const atom_t atom)
{
  X(lisp, atom);
}

/*
 * Symbol lookup.
 */

atom_t lisp_lookup(const lisp_t lisp, const atom_t closure, const atom_t atom);

/*
 * Lisp basic functions.
 */

ALWAYS_INLINE inline atom_t
lisp_car(const lisp_t lisp, const atom_t cell)
{
  if (likely(IS_PAIR(cell))) {
    return UP(CAR(cell));
  }
  return lisp_make_nil(lisp);
}

ALWAYS_INLINE inline atom_t
lisp_cdr(const lisp_t lisp, const atom_t cell)
{
  if (likely(IS_PAIR(cell))) {
    return UP(CDR(cell));
  }
  return lisp_make_nil(lisp);
}

/*
 * Internal list construction functions. CONS is pure, CONC is destructive.
 * NOTE Both functions consume their arguments.
 */

ALWAYS_INLINE inline atom_t
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

atom_t lisp_conc(const lisp_t lisp, const atom_t car, const atom_t cdr);

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
