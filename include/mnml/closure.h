#pragma once

#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <stdlib.h>

/*
 * Closure types.
 */

struct _closure;

typedef union _value
{
  int64_t number;
  atom_t atom;
  atom_t (*callback)(const lisp_t, const atom_t, struct _closure*,
                     union _value);
} value_t;

typedef struct _closure
{
  struct _closure* C;
  value_t V[];
} * closure_t;

typedef atom_t (*callback_t)(const lisp_t, const atom_t, closure_t, value_t);

/*
 * Closure operations.
 */

closure_t lisp_closure_get(closure_t* const $, const closure_t C,
                           const size_t N);

void lisp_closure_put(closure_t* const $, closure_t C);
void lisp_closure_clear(closure_t* const cache);

// vim: tw=80:sw=2:ts=2:sts=2:et
