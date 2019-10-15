#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <signal.h>
#include <unistd.h>

static atom_t
lisp_function_quit(const atom_t closure, const atom_t cell)
{
  kill(getpid(), SIGQUIT);
  X(cell);
  return UP(TRUE);
}

LISP_PLUGIN_REGISTER(quit, quit)
