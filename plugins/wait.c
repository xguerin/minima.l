#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>
#include <unistd.h>

atom_t
lisp_function_wait(const atom_t closure, const atom_t cell)
{
  atom_t car = lisp_eval(closure, lisp_car(cell));
  X(cell);
  pid_t pid = waitpid(car->number, NULL, 0);
  return lisp_make_number(pid);
}

LISP_REGISTER(wait, wait)
