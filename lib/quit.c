#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <signal.h>
#include <unistd.h>

static atom_t
lisp_function_quit(const atom_t closure, const atom_t arguments)
{
  kill(getpid(), SIGQUIT);
  return UP(TRUE);
}

LISP_PLUGIN_REGISTER(quit, quit)
