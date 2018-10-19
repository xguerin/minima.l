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
  T_NIL,
  T_NUMBER,
  T_STRING,
  T_SYMBOL,
  T_SYMBOL_INLINE,
  T_LIST
}
cell_type_t;

#define TYPE_MASK 0xFULL
#define RCNT_MASK 0xFFFF000000000000ULL

#define INLINE_SYMBOL_LEN 6

#define SET_TYPE(__e, __t) __e = ((__e & ~TYPE_MASK) | (               __t        &  TYPE_MASK))
#define SET_NUMB(__e, __v) __e = ((__e &  TYPE_MASK) | ( ((uintptr_t  )__v << 4)  & ~TYPE_MASK))
#define SET_SYMB(__e, __v) __e = ((__e &  TYPE_MASK) | ((*(uintptr_t *)__v << 8)  & ~TYPE_MASK))
#define SET_DATA(__e, __v) __e = ((__e &  TYPE_MASK) | (  (uintptr_t  )__v        & ~TYPE_MASK))

#define GET_TYPE(__e)      ((cell_type_t)  (__e &  TYPE_MASK             )      )
#define GET_NUMB(__e)      ((uint64_t   ) ((__e & ~TYPE_MASK             ) >> 4))
#define GET_SYMB(__e)      (((char *    )  &__e                            +  1))
#define GET_DATA(__e)      ((uint64_t   )  (__e & ~TYPE_MASK             )      )
#define GET_PNTR(__T, __e) ((__T        )  (__e & ~TYPE_MASK & ~RCNT_MASK)      )

#define IS_NULL(__c) (GET_TYPE(__c->car) == T_NIL)
#define IS_NUMB(__c) (GET_TYPE(__c->car) == T_NUMBER)
#define IS_LIST(__c) (GET_TYPE(__c->car) == T_LIST)

typedef struct _cell_t
{
  uintptr_t car;
  uintptr_t cdr;
}
* cell_t;

#define FOREACH(__c, __e)                               \
  for (cell_t __e = __c; GET_TYPE(__e->cdr) == T_LIST;  \
       __e = GET_PNTR(cell_t, __e->cdr))

/*
 * Interpreter statistics.
 */

void lisp_stats_print(FILE * fp);
bool lisp_stats_balanced_allocs();

/*
 * Function types.
 */

#define FUNCTION_TABLE_LEN 96
#define FUNCTION_TABLE_LVL 5

typedef cell_t (* function_t)(const cell_t cell);

typedef struct _function_entry_t
{
  function_t  fun;
  struct _function_entry_t *  table;
}
function_entry_t;

function_t lisp_function_lookup(const char * const sym);
bool lisp_function_register(const char * const sym, function_t fun);

/*
 * Consumer type.
 */

typedef void (* lisp_consumer_t)(const cell_t);

/*
 * Lisp basic functions.
 */

cell_t lisp_dup(const cell_t cell);
cell_t lisp_car(const cell_t cell);
cell_t lisp_cdr(const cell_t cell);
size_t lisp_len(const cell_t cell);

/*
 * Internal list construction functions. CONS is pure, CONC is destructive.
 */

bool   lisp_equl(const cell_t a, const cell_t b);
bool   lisp_eval(const cell_t a, cell_t * const r);
cell_t lisp_cons(const cell_t a, const cell_t b);
cell_t lisp_conc(const cell_t a, const cell_t b);

/*
 * Helper functions.
 */

cell_t lisp_make_nil();
cell_t lisp_make_number(const uint64_t num);
cell_t lisp_make_string(const char * const str);
cell_t lisp_make_symbol(const char * const sym);
cell_t lisp_make_list(const cell_t cell);
cell_t lisp_make_slot(const uintptr_t slot);

void lisp_free(const size_t n, ...);
void lisp_print(FILE * const fp, const cell_t cell);
