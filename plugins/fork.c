#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

atom_t
lisp_function_fork(const atom_t closure, const atom_t cell)
{
  X(cell);
  pid_t pid = fork();
  return lisp_make_number(pid < 0 ? errno : pid);
}

LISP_PLUGIN_REGISTER(fork, fork)
