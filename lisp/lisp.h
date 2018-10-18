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
  T_NUMBER,
  T_STRING,
  T_SYMBOL,
  T_SYMBOL_INLINE,
  T_LIST,
  T_NIL
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

/*
 * Function types.
 */

#define FUNCTION_TABLE_LEN 96
#define FUNCTION_TABLE_LVL 3

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
 * Helper functions.
 */

cell_t lisp_make_nil();
cell_t lisp_make_number(const uint64_t num);
cell_t lisp_make_string(const char * const str);
cell_t lisp_make_symbol(const char * const sym);
cell_t lisp_make_list(const cell_t cell);

void lisp_free(const cell_t cell);
void lisp_print(FILE * const fp, const cell_t cell);
