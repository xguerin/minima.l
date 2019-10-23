#pragma once

#include <mnml/types.h>
#include <string.h>

/*
 * Symbol makers.
 */

#define MAKE_SYMBOL_STATIC(__v, __s, __n)                         \
  symbol_t __v = (union _symbol*) alloca(sizeof(union _symbol));  \
  __v->word[0] = 0;                                               \
  __v->word[1] = 0;                                               \
  strncpy(__v->val, __s, __n);

#define MAKE_SYMBOL_DYNAMIC(__v, __s, __n)                        \
  symbol_t __v = (union _symbol*) malloc(sizeof(union _symbol));  \
  __v->word[0] = 0;                                               \
  __v->word[1] = 0;                                               \
  strncpy(__v->val, __s, __n);

/*
 * Function make/split.
 */

#define MAKE_FUNCTION(_f, _arg, _clo, _bdy)         \
  atom_t _f ## _bdy = lisp_cons(_bdy, NIL);         \
  X(_bdy); X(NIL);                                  \
  atom_t _f ## _clo = lisp_cons(_clo, _f ## _bdy);  \
  X(_clo); X(_f ## _bdy);                           \
  atom_t _f = lisp_cons(_arg, _f ## _clo);          \
  X(_arg); X(_f ## _clo);

#define SPLIT_FUNCTION(_f, _arg, _clo, _bdy)  \
  atom_t       _arg = lisp_car(_f);           \
  atom_t _f ## _cd0 = lisp_cdr(_f);           \
  X(_f);                                      \
  atom_t       _clo = lisp_car(_f ## _cd0);   \
  atom_t _f ## _cd1 = lisp_cdr(_f ## _cd0);   \
  X(_f ## _cd0);                              \
  atom_t       _bdy = lisp_car(_f ## _cd1);   \
  X(_f ## _cd1);

/*
 * Lambda make/split.
 */

#define MAKE_LAMBDA(_l, _sym, _arg, _bdy)             \
  atom_t _l ## _body  = lisp_cons(_bdy, NIL);         \
  X(_bdy); X(NIL);                                    \
  atom_t _l ## _args = lisp_cons(_arg, _l ## _body);  \
  X(_arg); X(_l ## _body);                            \
  atom_t _l = lisp_cons(_sym, _l ## _args);           \
  X(_sym); X(_l ## _args);

#define SPLIT_LAMBDA(_l, _sym, _arg, _bdy)  \
  atom_t       _sym = lisp_car(_l);         \
  atom_t _l ## _cd0 = lisp_cdr(_l);         \
  X(_l);                                    \
  atom_t       _arg = lisp_car(_l ## _cd0); \
  atom_t _l ## _cd1 = lisp_cdr(_l ## _cd0); \
  X(_l ## _cd0);                            \
  atom_t       _bdy = lisp_car(_l ## _cd1); \
  X(_l ## _cd1);

/*
 * CONS helpers.
 */

#define LISP_CONS_0(R) atom_t R = NIL

#define LISP_CONS_1(R, _1) LISP_CONS_0(R);      \
{                                               \
  atom_t r_1;                                   \
  if (strcmp(#_1, "@") == 0) {                  \
    MAKE_SYMBOL_STATIC(s_1, "@", 1);            \
    r_1 = lisp_make_symbol(s_1);                \
  } else {                                      \
    MAKE_SYMBOL_STATIC(s_1, #_1, strlen(#_1));  \
    atom_t __1 = lisp_make_symbol(s_1);         \
    r_1 = lisp_cons(__1, R);                    \
    X(__1); X(R);                               \
  }                                             \
  R = r_1;                                      \
}

#define LISP_CONS_2(R, _2, _1) LISP_CONS_0(R);  \
{                                               \
  MAKE_SYMBOL_STATIC(s_2, #_2, strlen(#_2));    \
  atom_t __2 = lisp_make_symbol(s_2);           \
  atom_t r_2;                                   \
  if (strcmp(#_1, "NIL") == 0) {                \
    r_2 = lisp_cons(__2, R);                    \
    X(R);                                       \
  } else {                                      \
    MAKE_SYMBOL_STATIC(s_1, #_1, strlen(#_1));  \
    atom_t __1 = lisp_make_symbol(s_1);         \
    r_2 = lisp_cons(__2, __1);                  \
    X(__1);                                     \
  }                                             \
  X(__2);                                       \
  R = r_2;                                      \
}

#define LISP_CONS_3(R, _3, ...) LISP_CONS_2(R, __VA_ARGS__);  \
{                                                           \
  MAKE_SYMBOL_STATIC(s_3, #_3, strlen(#_3));                \
  atom_t __3 = lisp_make_symbol(s_3);                       \
  atom_t r_3 = lisp_cons(__3, R);                           \
  X(__3); X(R);                                             \
  R = r_3;                                                  \
}

#define LISP_CONS_4(R, _4, ...) LISP_CONS_3(R, __VA_ARGS__);  \
{                                                             \
  MAKE_SYMBOL_STATIC(s_4, #_4, strlen(#_4));                  \
  atom_t __4 = lisp_make_symbol(s_4);                         \
  atom_t r_4 = lisp_cons(__4, R);                             \
  X(__4); X(R);                                               \
  R = r_4;                                                    \
}

#define LISP_CONS_5(R, _5, ...) LISP_CONS_4(R, __VA_ARGS__);  \
{                                                             \
  MAKE_SYMBOL_STATIC(s_5, #_5, strlen(#_5));                  \
  atom_t __5 = lisp_make_symbol(s_5);                         \
  atom_t r_5 = lisp_cons(__5, R);                             \
  X(__5); X(R);                                               \
  R = r_5;                                                    \
}

#define LISP_CONS_(_0, _1, _2, _3, _4, _5, NAME, ...) NAME

#define LISP_CONS(...)                                \
  LISP_CONS_(__VA_ARGS__, LISP_CONS_5,  LISP_CONS_4,  \
             LISP_CONS_3,  LISP_CONS_2,  LISP_CONS_1, \
             LISP_CONS_0)(__VA_ARGS__)

/*
 * Atom makers.
 */

atom_t lisp_make_char(const char c);
atom_t lisp_make_number(const int64_t num);
atom_t lisp_make_string(const char * const s, const size_t len);
atom_t lisp_make_symbol(const symbol_t sym);

/*
 * Singleton makers.
 */

void lisp_make_nil();
void lisp_make_true();
void lisp_make_quote();
void lisp_make_wildcard();

// vim: tw=80:sw=2:ts=2:sts=2:et
