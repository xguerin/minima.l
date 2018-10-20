#pragma once

#include "lisp.h"

/*
 * Symbol types.
 */

#define SYMBOL_TABLE_LEN 96
#define SYMBOL_TABLE_LVL 5

typedef cell_t (* function_t)(const cell_t cell);

#define EVAL_FLAG (0x1ULL)
#define FUNC_FLAG (0x2ULL)

#define SET_FUNC(__e, __v) __e = (__e | (__v ? FUNC_FLAG : 0))
#define SET_EVAL(__e, __v) __e = (__e | (__v ? EVAL_FLAG : 0))

#define IS_FUNC(__e) (__e & FUNC_FLAG)
#define IS_EVAL(__e) (__e & EVAL_FLAG)

#define MAKE_SYMBOL(__p, __f) ((((uintptr_t)__p) & PNTR_MASK) | __f)

typedef struct _symbol_entry_t
{
  uintptr_t                sym;
  struct _symbol_entry_t * table;
}
symbol_entry_t;

bool lisp_symbol_register(const char * const sym, const uintptr_t symbol);
uintptr_t lisp_symbol_lookup(const char * const sym);
