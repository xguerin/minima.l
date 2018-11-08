#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Lisp types.
 */

typedef enum _atom_type
{
  T_NIL,
  T_TRUE,
  T_NUMBER,
  T_PAIR,
  T_STRING,
  T_SYMBOL,
  T_INLINE,
  T_WILDCARD
}
atom_type_t;

#define ATOM_TYPES 8

struct _atom;

typedef struct _pair
{
  struct _atom * car;
  struct _atom * cdr;
}
* pair_t;

typedef struct _atom
{
  uint32_t        next;
  atom_type_t     type :32;
  uint64_t        refs;
  union {
    int64_t       number;
    const char *  string;
    char          symbol[8];
    uint64_t      tag;
    struct _pair  pair;
  };
}
__attribute__((packed)) * atom_t;

#define IS_NULL(__a) ((__a)->type == T_NIL)
#define IS_TRUE(__a) ((__a)->type == T_TRUE)
#define IS_NUMB(__a) ((__a)->type == T_NUMBER)
#define IS_PAIR(__a) ((__a)->type == T_PAIR)
#define IS_STRN(__a) ((__a)->type == T_STRING)
#define IS_SYMB(__a) ((__a)->type == T_SYMBOL || (__a->type == T_INLINE))

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
extern atom_t NIL;
extern atom_t TRUE;
extern atom_t WILDCARD;

/*
 * Lisp basic functions.
 */

atom_t lisp_dup(const atom_t cell);
atom_t lisp_car(const atom_t cell);
atom_t lisp_cdr(const atom_t cell);

/*
 * Internal list construction functions. CONS is pure, CONC is destructive.
 */

bool   lisp_equl(const atom_t a, const atom_t b);
atom_t lisp_cons(const atom_t a, const atom_t b);
atom_t lisp_conc(const atom_t a, const atom_t b);

/*
 * Evaluation and closure functions.
 */

atom_t lisp_bind(const atom_t closure, const atom_t args, const atom_t vals);
atom_t lisp_prog(const atom_t closure, const atom_t cell, const atom_t rslt);
atom_t lisp_eval(const atom_t closure, const atom_t cell);
atom_t lisp_setq(const atom_t closure, const atom_t sym, const atom_t val);

/*
 * Helper functions.
 */

void lisp_make_nil();
void lisp_make_true();
void lisp_make_wildcard();

atom_t lisp_make_number(const int64_t num);
atom_t lisp_make_inline(const uint64_t tag);
atom_t lisp_make_string(const char * const str);
atom_t lisp_make_symbol(const char * const sym);

void lisp_print(FILE * const fp, const atom_t cell);

/*
 * Debug.
 */

#ifdef LISP_ENABLE_DEBUG

#define TRACE(__fmt, ...) {                                             \
  printf("! %s:%d: " __fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__);  \
}

#ifdef __MACH__

#define HEADER_SEXP(__c)  \
  printf("! %s:%d: [%llu] %s = ", __FUNCTION__, __LINE__, __c->refs, #__c)

#else

#define HEADER_SEXP(__c)  \
  printf("! %s:%d: [%lu] %s = ", __FUNCTION__, __LINE__, __c->refs, #__c)

#endif

#define TRACE_SEXP(__c) {   \
  HEADER_SEXP(__c);         \
  lisp_print(stdout, __c);  \
}

#else

#define TRACE(__fmt, ...)
#define TRACE_SEXP(__c)

#endif
