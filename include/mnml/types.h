#pragma once

#include <stdint.h>

#ifdef LISP_ENABLE_SSE
#include <smmintrin.h>
#endif

/*
 * Global types.
 */

#ifdef LISP_ENABLE_SSE
typedef __m128i int128_t;
#else
typedef __int128 int128_t;
#endif

/*
 * Optimization macros.
 */

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/*
 * Lisp types.
 */

typedef enum atom_type
{
  T_NONE,
  T_NIL,
  T_TRUE,
  T_CHAR,
  T_NUMBER,
  T_PAIR,
  T_SYMBOL,
  T_WILDCARD
} atom_type_t;

typedef enum atom_flag
{
  F_TAIL_CALL = 0x1,
  F_HAS_COLOR = 0x2,
  F_WEAKREF = 0x4,
} atom_flag_t;

#define ATOM_TYPES 7

struct atom;

typedef struct pair
{
  struct atom* car;
  struct atom* cdr;
} * pair_t;

#define LISP_SYMBOL_LENGTH 16

typedef union symbol
{
  char val[LISP_SYMBOL_LENGTH];
  int128_t tag;
} __attribute__((packed)) * symbol_t;

#ifdef LISP_ENABLE_SSE
#define NULL_TAG _mm_setzero_si128()
#else
#define NULL_TAG 0
#endif

typedef struct atom
{
  union
  {
    uint32_t next;
    uint32_t refs;
  };
  atom_type_t type : 16;
  uint16_t flags;
  struct atom* cache;
  union
  {
    int64_t number;
    union symbol symbol;
    struct pair pair;
  };
} __attribute__((packed)) * atom_t;

#define CAR(__a) ((__a)->pair.car)
#define CDR(__a) ((__a)->pair.cdr)

#define IS_NULL(__a) ((__a)->type == T_NIL)
#define IS_TRUE(__a) ((__a)->type == T_TRUE)
#define IS_WILD(__a) ((__a)->type == T_WILDCARD)
#define IS_CHAR(__a) ((__a)->type == T_CHAR)
#define IS_NUMB(__a) ((__a)->type == T_NUMBER)
#define IS_PAIR(__a) ((__a)->type == T_PAIR)
#define IS_SYMB(__a) ((__a)->type == T_SYMBOL)

#define IS_LIST(__a) (IS_PAIR(__a) || IS_NULL(__a))
#define IS_ATOM(__a) (!IS_LIST(__a))

/*
 * Flags.
 */

#define IS_TAIL_CALL(__a) (((__a)->flags & F_TAIL_CALL) == F_TAIL_CALL)
#define IS_COLORED(__a) (((__a)->flags & F_HAS_COLOR) == F_HAS_COLOR)
#define IS_WEAKREF(__a) (((__a)->flags & F_WEAKREF) == F_WEAKREF)

#define SET_TAIL_CALL(__a) ((__a)->flags |= F_TAIL_CALL)
#define CLR_TAIL_CALL(__a) ((__a)->flags &= ~F_TAIL_CALL)

#define SET_COLOR(__a) ((__a)->flags |= F_HAS_COLOR)
#define CLR_COLOR(__a) ((__a)->flags &= ~F_HAS_COLOR)

#define SET_WEAKREF(__a) ((__a)->flags |= F_WEAKREF)
#define CLR_WEAKREF(__a) ((__a)->flags &= ~F_WEAKREF)

/*
 * A function has the following format:
 * (ARGS CLOSURE BODY)
 */

#define IS_ARGS(__a) (IS_PAIR(__a) && (IS_LIST(CAR(__a)) || IS_SYMB(CAR(__a))))
#define IS_CLOS(__a) (!IS_NULL(__a) && IS_PAIR(__a) && IS_LIST(CAR(__a)))
#define IS_BODY(__a) (!IS_NULL(__a) && (IS_PAIR(__a) || IS_NUMB(__a)))

#define IS_FUNC(__a) \
  (IS_ARGS(__a) && IS_CLOS(CDR(__a)) && IS_BODY(CDR(CDR(__a))))

// vim: tw=80:sw=2:ts=2:sts=2:et
