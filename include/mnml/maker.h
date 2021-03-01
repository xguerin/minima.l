#pragma once

#include <mnml/lisp.h>
#include <mnml/types.h>
#include <string.h>

/*
 * Symbol makers.
 */

#define MAKE_SYMBOL_STATIC(__v, __s, __n)                       \
  symbol_t __v = (union symbol*)alloca(sizeof(union symbol)); \
  (__v)->word[0] = 0;                                           \
  (__v)->word[1] = 0;                                           \
  strncpy((__v)->val, __s, __n)

#define MAKE_SYMBOL_DYNAMIC(__v, __s, __n)                      \
  symbol_t __v = (union symbol*)malloc(sizeof(union symbol)); \
  (__v)->word[0] = 0;                                           \
  (__v)->word[1] = 0;                                           \
  strncpy((__v)->val, __s, __n)

/*
 * CONS helpers.
 */

#define LISP_CONS_0(__l, __r) atom_t __r = lisp_make_nil(__l)

#define LISP_CONS_1(__l, __r, _1)              \
  LISP_CONS_0(__l, __r);                       \
  {                                            \
    MAKE_SYMBOL_STATIC(s_1, #_1, strlen(#_1)); \
    atom_t r_1 = lisp_make_symbol(__l, s_1);   \
    X((__l)->slab, __r);                       \
    (__r) = r_1;                               \
  }

#define LISP_CONS_NIL(__l, __r, _1)            \
  LISP_CONS_0(__l, __r);                       \
  {                                            \
    MAKE_SYMBOL_STATIC(s_1, #_1, strlen(#_1)); \
    atom_t __1 = lisp_make_symbol(__l, s_1);   \
    (__r) = lisp_cons(__l, __1, __r);          \
  }

#define LISP_CONS_REM(__l, __r, _1)            \
  LISP_CONS_0(__l, __r);                       \
  {                                            \
    X((__l)->slab, __r);                       \
    MAKE_SYMBOL_STATIC(s_1, #_1, strlen(#_1)); \
    atom_t __1 = lisp_make_symbol(__l, s_1);   \
    MAKE_SYMBOL_STATIC(s_0, "REM", 3);         \
    atom_t __0 = lisp_make_symbol(__l, s_0);   \
    (__r) = lisp_cons(__l, __1, __0);          \
  }

#define LISP_CONS_2(__l, __r, _2, _1) LISP_CONS_##_1(__l, __r, _2)

#define LISP_CONS_3(__l, __r, _3, ...)         \
  LISP_CONS_2(__l, __r, __VA_ARGS__);          \
  {                                            \
    MAKE_SYMBOL_STATIC(s_3, #_3, strlen(#_3)); \
    atom_t __3 = lisp_make_symbol(__l, s_3);   \
    __r = lisp_cons(__l, __3, __r);            \
  }

#define LISP_CONS_4(__l, __r, _4, ...)         \
  LISP_CONS_3(__l, __r, __VA_ARGS__);          \
  {                                            \
    MAKE_SYMBOL_STATIC(s_4, #_4, strlen(#_4)); \
    atom_t __4 = lisp_make_symbol(__l, s_4);   \
    __r = lisp_cons(__l, __4, __r);            \
  }

#define LISP_CONS_5(__l, __r, _5, ...)         \
  LISP_CONS_4(__l, __r, __VA_ARGS__);          \
  {                                            \
    MAKE_SYMBOL_STATIC(s_5, #_5, strlen(#_5)); \
    atom_t __5 = lisp_make_symbol(__l, s_5);   \
    __r = lisp_cons(__l, __5, __r);            \
  }

#define LISP_CONS_(_1, _2, _3, _4, _5, NAME, ...) NAME

#define LISP_CONS(__l, __r, ...)                                              \
  LISP_CONS_(__VA_ARGS__, LISP_CONS_5, LISP_CONS_4, LISP_CONS_3, LISP_CONS_2, \
             LISP_CONS_1)                                                     \
  (__l, __r, __VA_ARGS__)

/*
 * Atom makers.
 */

atom_t lisp_make_char(const lisp_t lisp, const char c);
atom_t lisp_make_number(const lisp_t lisp, const int64_t num);
atom_t lisp_make_string(const lisp_t lisp, const char* const s,
                        const size_t len);
atom_t lisp_make_symbol(const lisp_t lisp, const symbol_t sym);

atom_t lisp_make_nil(const lisp_t lisp);
atom_t lisp_make_true(const lisp_t lisp);
atom_t lisp_make_quote(const lisp_t lisp);
atom_t lisp_make_wildcard(const lisp_t lisp);

// vim: tw=80:sw=2:ts=2:sts=2:et
