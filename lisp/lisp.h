#pragma once

#include <stdbool.h>
#include <stdint.h>

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
  T_CELL,
  T_NIL
}
cell_type_t;

#define MASK 0xFULL

#define SET_TYPE(__e, __t) __e = ((__e & ~MASK) | (               __t       &  MASK))
#define SET_NUMB(__e, __v) __e = ((__e &  MASK) | ( ((uintptr_t  )__v << 4) & ~MASK))
#define SET_SYMB(__e, __v) __e = ((__e &  MASK) | ((*(uintptr_t *)__v << 8) & ~MASK))
#define SET_DATA(__e, __v) __e = ((__e &  MASK) | (  (uintptr_t  )__v       & ~MASK))

#define GET_TYPE(__e)      ((cell_type_t)  (__e &  MASK)      )
#define GET_NUMB(__e)      ((uint64_t   ) ((__e & ~MASK) >> 4))
#define GET_DATA(__e)      ((uint64_t   )  (__e & ~MASK)      )
#define GET_PNTR(__T, __e) ((__T        )  (__e & ~MASK)      )
#define GET_SYMB(__e)      (((char *    )  &__e          +  1))

typedef struct _cell_t
{
  uintptr_t car;
  uintptr_t cdr;
}
* cell_t;

/*
 * Consumer type.
 */

typedef void (* lisp_consumer_t)(const cell_t);

/*
 * Helper functions.
 */

void lisp_free(const cell_t cell);
void lisp_print(const cell_t cell);
