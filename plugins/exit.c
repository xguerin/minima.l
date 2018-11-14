#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
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
