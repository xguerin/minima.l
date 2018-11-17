#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>
#include <unistd.h>

atom_t
lisp_function_fork(const atom_t closure, const atom_t cell)
{
  X(cell);
  pid_t pid = fork();
  return lisp_make_number(pid);
}

LISP_REGISTER(fork, fork)
