#include <mnml/closure.h>
#include <stdlib.h>
#include <string.h>

#define MALLOC_SIZE (sizeof(closure_t) + CLOSURE_SIZE * sizeof(atom_t))

closure_t
lisp_closure_allocate(closure_t C)
{
  closure_t result = (closure_t)malloc(MALLOC_SIZE);
  memset(result, 0, MALLOC_SIZE);
  result->C = C;
  return result;
}

void
lisp_closure_deallocate(const closure_t closure)
{
  free(closure);
}
