#include <mnml/closure.h>
#include <string.h>

closure_t
lisp_closure_allocate(closure_t C)
{
  size_t len = sizeof(closure_t) + sizeof(atom_t) * CLOSURE_SIZE;
  closure_t R = (closure_t)malloc(len);
  memset(R, 0, len);
  R->C = C;
  return R;
}

void
lisp_closure_deallocate(const closure_t closure)
{
  free(closure);
}
