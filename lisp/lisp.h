#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Lisp types.
 */

struct _cell_t;

typedef enum _cell_type_t
{
  T_NIL           = 0,
  T_TRUE          = 1,
  T_NUMBER        = 2,
  T_STRING        = 3,
  T_SYMBOL        = 4,
  T_SYMBOL_INLINE = 5,
  T_LIST          = 6
}
cell_type_t;

#define TYPE_MASK 0xFULL
#define PNTR_MASK ~TYPE_MASK
#define INLINE_SYMBOL_LEN 6

#define SET_TYPE(__e, __t) __e = ((__e & PNTR_MASK) | (               __t        & TYPE_MASK))
#define SET_NUMB(__e, __v) __e = ((__e & TYPE_MASK) | ( ((uintptr_t  )__v << 4)  & PNTR_MASK))
#define SET_SYMB(__e, __v) __e = ((__e & TYPE_MASK) | ((*(uintptr_t *)__v << 8)  & PNTR_MASK))
#define SET_DATA(__e, __v) __e = ((__e & TYPE_MASK) | (  (uintptr_t  )__v        & PNTR_MASK))

#define GET_TYPE(__e)      ((cell_type_t)  (__e & TYPE_MASK)      )
#define GET_NUMB(__e)      ((uint64_t   ) ((__e & PNTR_MASK) >> 4))
#define GET_SYMB(__e)      (((char *    )  &__e              +  1))
#define GET_DATA(__e)      ((uint64_t   )  (__e & PNTR_MASK)      )
#define GET_PNTR(__T, __e) ((__T        )  (__e & PNTR_MASK)      )

#define IS_NULL(__c) (GET_TYPE(__c->car) == T_NIL)
#define IS_NUMB(__c) (GET_TYPE(__c->car) == T_NUMBER)
#define IS_STRN(__c) (GET_TYPE(__c->car) == T_STRING)
#define IS_SYMB(__c) (GET_TYPE(__c->car) == T_SYMBOL || GET_TYPE(__c->car) == T_SYMBOL_INLINE)
#define IS_LIST(__c) (GET_TYPE(__c->car) == T_LIST)

typedef struct _cell_t
{
  uintptr_t car;
  uintptr_t cdr;
}
* cell_t;

/*
 * Helper macros.
 */

#define FOREACH(__c, __p)                   \
  cell_t __p = GET_PNTR(cell_t, __c->car);  \
  for (;;)

#define NEXT(__p)                           \
  if (GET_TYPE(__p->cdr) != T_LIST) break;  \
  __p = GET_PNTR(cell_t, __p->cdr)

/*
 * Consumer type.
 */

typedef void (* lisp_consumer_t)(const cell_t);

/*
 * Symbol management.
 */

extern cell_t globals;

char * lisp_get_sym(const cell_t cell);
cell_t lisp_lookup(const cell_t sym);

/*
 * Lisp basic functions.
 */

cell_t lisp_dup(const cell_t cell);
cell_t lisp_car(const cell_t cell);
cell_t lisp_cdr(const cell_t cell);

/*
 * Internal list construction functions. CONS is pure, CONC is destructive.
 */

bool   lisp_equl(const cell_t a, const cell_t b);
cell_t lisp_cons(const cell_t a, const cell_t b);
cell_t lisp_conc(const cell_t a, const cell_t b);
cell_t lisp_eval(const cell_t cell);

/*
 * Helper functions.
 */

cell_t lisp_make_nil();
cell_t lisp_make_true();
cell_t lisp_make_number(const uint64_t num);
cell_t lisp_make_string(const char * const str);
cell_t lisp_make_symbol(const char * const sym);
cell_t lisp_make_list(const cell_t cell);
cell_t lisp_make_slot(const uintptr_t slot);

void lisp_print(FILE * const fp, const cell_t cell);

/*
 * Debug.
 */

#ifdef LISP_DEBUG
#define TRACE(__c) {                           \
  printf("! %s:%d: ", __FUNCTION__, __LINE__); \
  lisp_print(stdout, __c);                     \
}
#else
#define TRACE(__c)
#endif
