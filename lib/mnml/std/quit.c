#include <mnml/module.h>
#include <mnml/slab.h>
#include <signal.h>
#include <unistd.h>

static atom_t
lisp_function_quit(const lisp_t lisp, const atom_t closure)
{
  kill(getpid(), SIGQUIT);
  return UP(TRUE);
}

LISP_MODULE_SETUP(quit, quit)

// vim: tw=80:sw=2:ts=2:sts=2:et
