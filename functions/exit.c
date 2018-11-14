#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <signal.h>
#include <string.h>

atom_t
lisp_function_exit(const atom_t closure, const atom_t cell)
{
  raise(SIGTERM);
  X(cell);
  return UP(TRUE);
}

LISP_REGISTER(exit, exit)
