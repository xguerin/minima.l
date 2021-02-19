#include <mnml/module.h>
#include <mnml/slab.h>
#include <signal.h>
#include <unistd.h>

static atom_t USED
lisp_function_quit(UNUSED const lisp_t lisp, UNUSED const atom_t closure)
{
  kill(getpid(), SIGQUIT);
  return lisp_make_true();
}

LISP_MODULE_SETUP(quit, quit)

// vim: tw=80:sw=2:ts=2:sts=2:et
