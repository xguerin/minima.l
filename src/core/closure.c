#include <mnml/closure.h>
#include <mnml/slab.h>
#include <stdlib.h>
#include <string.h>

#define MALLOC_SIZE(_N) (sizeof(closure_t) + (_N) * sizeof(atom_t))

closure_t
lisp_closure_get(closure_t* const $, const closure_t C, const size_t N)
{
  closure_t result;
  /*
   * Allocate the new closure, preferably from the cache.
   */
  if (*$ != NULL) {
    result = *$;
    *$ = result->C;
  } else {
    result = (closure_t)malloc(MALLOC_SIZE(N));
  }
  /*
   * Return the closure.
   */
  result->C = C;
  return result;
}

void
lisp_closure_put(closure_t* const $, const closure_t C)
{
  C->C = *$;
  *$ = C;
}

void
lisp_closure_clear(closure_t* $)
{
  while (*$ != NULL) {
    closure_t tmp = *$;
    *$ = tmp->C;
    free(tmp);
  }
}

// vim: tw=80:sw=2:ts=2:sts=2:et
