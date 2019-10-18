#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <errno.h>
#include <unistd.h>

static atom_t
lisp_function_fork(const atom_t closure, const atom_t arguments)
{
  pid_t pid = fork();
  return lisp_make_number(pid < 0 ? errno : pid);
}

LISP_PLUGIN_REGISTER(fork, fork)
