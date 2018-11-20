#pragma once

#include "types.h"
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
 * Consumer type.
 */

typedef void (* lisp_consumer_t)(const atom_t);

/*
 * Symbol management.
 */

extern atom_t GLOBALS;
extern atom_t PLUGINS;
extern atom_t ICHAN;
extern atom_t OCHAN;
extern atom_t NIL;
extern atom_t TRUE;
extern atom_t WILDCARD;

atom_t lisp_lookup(const atom_t closure, const atom_t sym);

/*
 * Interpreter life cycle.
 */

typedef void (* error_handler_t)();

void lisp_set_parse_error_handler(const error_handler_t h);
void lisp_set_syntax_error_handler(const error_handler_t h);

void lisp_init();
void lisp_fini();

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

atom_t lisp_bind(const atom_t closure, const atom_t args, const atom_t vals);
atom_t lisp_setq(const atom_t closure, const atom_t sym, const atom_t val);
atom_t lisp_prog(const atom_t closure, const atom_t cell, const atom_t rslt);

atom_t lisp_read(const atom_t closure, const atom_t cell);
atom_t lisp_eval(const atom_t closure, const atom_t cell);
void   lisp_prin(const atom_t closure, const atom_t cell, const bool s);

/*
 * Helper functions.
 */

void lisp_make_nil();
void lisp_make_true();
void lisp_make_wildcard();

atom_t lisp_make_number(const int64_t num);
atom_t lisp_make_inline(const uint64_t tag);
atom_t lisp_make_char(const char c);
atom_t lisp_make_string(const char * const s, const size_t len);
atom_t lisp_make_symbol(const symbol_t sym);

#define PUSH_IO_CONTEXT(__c, __d) { \
  atom_t n = lisp_make_number(__d); \
  atom_t l = lisp_cons(n, NIL);     \
  atom_t o = __c;                   \
  __c = lisp_cons(l, o);            \
  X(o); X(l); X(n);                 \
}

#define POP_IO_CONTEXT(__c) { \
  atom_t old = __c;           \
  __c = UP(CDR(__c));         \
  X(old);                     \
}

/*
 * Symbol matching.
 */

#define MAKE_SYMBOL_STATIC(__v, __s, __n)       \
  symbol_t __v = alloca(sizeof(union _symbol)); \
  __v->word[0] = 0;                             \
  __v->word[1] = 0;                             \
  strncpy(__v->val, __s, __n);

#define MAKE_SYMBOL_DYNAMIC(__v, __s, __n)      \
  symbol_t __v = malloc(sizeof(union _symbol)); \
  __v->word[0] = 0;                             \
  __v->word[1] = 0;                             \
  strncpy(__v->val, __s, __n);

static inline bool
lisp_symbol_match(const atom_t a, const atom_t b)
{
  register __m128i res = _mm_xor_si128(a->symbol.tag, b->symbol.tag);
  return _mm_test_all_zeros(res, res);
}
