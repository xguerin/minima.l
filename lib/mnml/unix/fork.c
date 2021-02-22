#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <errno.h>
#include <unistd.h>

static atom_t USED
lisp_function_fork(const lisp_t lisp, UNUSED const atom_t closure)
{
  pid_t pid = fork();
  return lisp_make_number(lisp, pid < 0 ? errno : pid);
}

LISP_MODULE_SETUP(fork, fork)

// vim: tw=80:sw=2:ts=2:sts=2:et
