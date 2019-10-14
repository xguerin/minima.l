#pragma once

#include <mnml/debug.h>
#include <mnml/lisp.h>

/*
 * Closure types.
 */

struct _closure;

typedef atom_t (*callback_t)(struct _closure * C, const atom_t V);

typedef struct _closure {
  struct _closure * C;
  atom_t V[];
}
* closure_t;

/*
 * Closure macros.
 */

#define CLOSURE_SIZE 15

/*
 * Closure functions.
 */

closure_t lisp_closure_allocate(closure_t C);
void lisp_closure_deallocate(const closure_t closure);
