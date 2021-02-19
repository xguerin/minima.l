#pragma once

#include <mnml/types.h>
#include <string.h>

/*
 * Symbol makers.
 */

#define MAKE_SYMBOL_STATIC(__v, __s, __n)                       \
  symbol_t __v = (union _symbol*)alloca(sizeof(union _symbol)); \
  __v->word[0] = 0;                                             \
  __v->word[1] = 0;                                             \
  strncpy(__v->val, __s, __n)

#define MAKE_SYMBOL_DYNAMIC(__v, __s, __n)                      \
  symbol_t __v = (union _symbol*)malloc(sizeof(union _symbol)); \
  __v->word[0] = 0;                                             \
  __v->word[1] = 0;                                             \
  strncpy(__v->val, __s, __n)

/*
 * CONS helpers.
 */

#define LISP_CONS_0(R) atom_t R = lisp_make_nil()

#define LISP_CONS_1(R, _1)                     \
  LISP_CONS_0(R);                              \
  {                                            \
    MAKE_SYMBOL_STATIC(s_1, #_1, strlen(#_1)); \
    atom_t r_1 = lisp_make_symbol(s_1);        \
    X(R);                                      \
    R = r_1;                                   \
  }

#define LISP_CONS_NIL(R, _1)                   \
  LISP_CONS_0(R);                              \
  {                                            \
    MAKE_SYMBOL_STATIC(s_1, #_1, strlen(#_1)); \
    atom_t __1 = lisp_make_symbol(s_1);        \
    R = lisp_cons(__1, R);                     \
  }

#define LISP_CONS_REM(R, _1)                   \
  LISP_CONS_0(R);                              \
  {                                            \
    X(R);                                      \
    MAKE_SYMBOL_STATIC(s_1, #_1, strlen(#_1)); \
    atom_t __1 = lisp_make_symbol(s_1);        \
    MAKE_SYMBOL_STATIC(s_0, "REM", 3);         \
    atom_t __0 = lisp_make_symbol(s_0);        \
    R = lisp_cons(__1, __0);                   \
  }

#define LISP_CONS_2(R, _2, _1) LISP_CONS_##_1(R, _2)

#define LISP_CONS_3(R, _3, ...)                \
  LISP_CONS_2(R, __VA_ARGS__);                 \
  {                                            \
    MAKE_SYMBOL_STATIC(s_3, #_3, strlen(#_3)); \
    atom_t __3 = lisp_make_symbol(s_3);        \
    R = lisp_cons(__3, R);                     \
  }

#define LISP_CONS_4(R, _4, ...)                \
  LISP_CONS_3(R, __VA_ARGS__);                 \
  {                                            \
    MAKE_SYMBOL_STATIC(s_4, #_4, strlen(#_4)); \
    atom_t __4 = lisp_make_symbol(s_4);        \
    R = lisp_cons(__4, R);                     \
  }

#define LISP_CONS_5(R, _5, ...)                \
  LISP_CONS_4(R, __VA_ARGS__);                 \
  {                                            \
    MAKE_SYMBOL_STATIC(s_5, #_5, strlen(#_5)); \
    atom_t __5 = lisp_make_symbol(s_5);        \
    R = lisp_cons(__5, R);                     \
  }

#define LISP_CONS_(_0, _1, _2, _3, _4, _5, NAME, ...) NAME

#define LISP_CONS(...)                                                        \
  LISP_CONS_(__VA_ARGS__, LISP_CONS_5, LISP_CONS_4, LISP_CONS_3, LISP_CONS_2, \
             LISP_CONS_1, LISP_CONS_0)                                        \
  (__VA_ARGS__)

/*
 * Atom makers.
 */

atom_t lisp_make_char(const char c);
atom_t lisp_make_number(const int64_t num);
atom_t lisp_make_string(const char* const s, const size_t len);
atom_t lisp_make_symbol(const symbol_t sym);

atom_t lisp_make_nil();
atom_t lisp_make_true();
atom_t lisp_make_quote();
atom_t lisp_make_wildcard();

// vim: tw=80:sw=2:ts=2:sts=2:et
