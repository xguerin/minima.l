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


