#include <mnml/closure.h>
#include <mnml/slab.h>
#include <stdlib.h>
#include <string.h>

#define MALLOC_SIZE (sizeof(closure_t) + CLOSURE_SIZE * sizeof(atom_t))

static closure_t closure_cache = NULL;

closure_t
lisp_closure_allocate(closure_t C)
{
  closure_t result;
  /*
   * Allocate the new closure, preferably from the cache.
   */
  if (closure_cache != NULL) {
    result = closure_cache;
    closure_cache = closure_cache->C;
  } else {
    result = (closure_t)malloc(MALLOC_SIZE);
  }
  /*
   * Reset the closure and return.
   */
  memset(result, 0, MALLOC_SIZE);
  result->C = C;
  return result;
}

void
lisp_closure_deallocate(const closure_t C)
{
  /*
   * Clean-up all lingering atoms.
   */
  for (size_t i = 0; i < CLOSURE_SIZE; i += 1) {
    if (unlikely(C->V[i] != NULL)) {
      X(C->V[i]);
      C->V[i] = NULL;
    }
  }
  /*
   * Prepend the closure to the list.
   */
  C->C = closure_cache;
  closure_cache = C;
}
