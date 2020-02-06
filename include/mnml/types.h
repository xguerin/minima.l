#pragma once

#include <stdint.h>

#ifdef LISP_ENABLE_SSE41
#include <smmintrin.h>
#endif

/*
 * Optimization macros.
 */

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/*
 * Lisp types.
 */

typedef enum _atom_type
{
  T_NIL,
  T_TRUE,
  T_CHAR,
  T_NUMBER,
  T_PAIR,
  T_SYMBOL,
  T_WILDCARD
}
atom_type_t;

#define ATOM_TYPES 7

struct _atom;

typedef struct _pair
{
  struct _atom * car;
  struct _atom * cdr;
}
* pair_t;

#define LISP_SYMBOL_LENGTH 16

typedef union _symbol
{
  char      val[LISP_SYMBOL_LENGTH];
  uint64_t  word[2];
#ifdef LISP_ENABLE_SSE41
  __m128i   tag;
#endif
}
__attribute__((packed)) * symbol_t;

typedef struct _atom
{
  uint32_t        next;
  atom_type_t     type :32;
  uint64_t        refs;
  union {
    int64_t       number;
    union _symbol symbol;
    struct _pair  pair;
  };
}
__attribute__((packed)) * atom_t;

#define CAR(__a) ((__a)->pair.car)
#define CDR(__a) ((__a)->pair.cdr)

#define IS_NULL(__a) ((__a)       ==   NIL)
#define IS_TRUE(__a) ((__a)       ==   TRUE)
#define IS_WILD(__a) ((__a)       ==   WILDCARD)
#define IS_CHAR(__a) ((__a)->type == T_CHAR)
#define IS_NUMB(__a) ((__a)->type == T_NUMBER)
#define IS_PAIR(__a) ((__a)->type == T_PAIR)
#define IS_SYMB(__a) ((__a)->type == T_SYMBOL)

#define IS_LIST(__a) (IS_PAIR(__a) || IS_NULL(__a))
#define IS_ATOM(__a) (!IS_LIST(__a))

/*
 * Lisp context type.
 */

typedef struct lisp
{
  atom_t GLOBALS;
}
* lisp_t;

/*
 * A function has the following format:
 * (ARGS CLOSURE CURY BODY)
 */

#define IS_FUNC(__a)                                                                \
  (IS_PAIR(__a) &&                                                                  \
   !IS_NULL(CDR(__a)) && !IS_NULL(CDR(CDR(__a))) && !IS_NULL(CDR(CDR(CDR(__a)))) && \
   (IS_LIST(CAR(__a)) || IS_SYMB(CAR(__a))) &&                                      \
   IS_LIST(CAR(CDR(__a))) &&                                                        \
   IS_LIST(CAR(CDR(CDR(__a)))))

typedef atom_t (* function_t)(const lisp_t, const atom_t, const atom_t);

// vim: tw=80:sw=2:ts=2:sts=2:et
