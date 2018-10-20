#pragma once

#include "lisp.h"

/*
 * Symbol types.
 */

#define SYMBOL_TABLE_LEN 96
#define SYMBOL_TABLE_LVL 5

typedef struct _symbol_entry_t
{
  cell_t                   cell;
  struct _symbol_entry_t * table;
}
symbol_entry_t;

bool lisp_symbol_register(const char * const sym, const cell_t cell);
cell_t lisp_symbol_lookup(const char * const sym);
