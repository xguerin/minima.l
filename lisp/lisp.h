#pragma once

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
  T_CELL,
  T_NIL
}
cell_type_t;

typedef union _value_t
{
  uint64_t          number;
  char *            string;
  char *            symbol;
  struct _cell_t *  cell;
}
cell_value_t;

typedef struct _cell_entry_t
{
  cell_type_t  type;
  cell_value_t value;
}
cell_entry_t;

typedef struct _cell_t
{
  cell_entry_t car;
  cell_entry_t cdr;
}
* cell_t;

/*
 * Helper functions.
 */

void lisp_print(const cell_t cell);
