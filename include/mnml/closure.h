#pragma once

#include <mnml/debug.h>
#include <mnml/lisp.h>

/*
 * Closure types.
 */

struct _closure;

typedef union _value {
  int64_t number;
  atom_t list;
  atom_t (*callback)(struct _closure * C, union _value V);
}
value_t;

typedef struct _closure {
  struct _closure * C;
  value_t V[];
}
* closure_t;

typedef atom_t (*callback_t)(closure_t C, value_t V);

/*
 * Closure operations.
 */

closure_t lisp_closure_get(closure_t* const $, const closure_t C, const size_t N);

void lisp_closure_put(closure_t* const $, closure_t C);
void lisp_closure_clear(closure_t* const cache);
