#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

static atom_t
lisp_function_wait(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(car, arguments, X);
  /*
   * Get the PID.
   */
  int tpid = car->number, state;
  X(car);
  /*
   * Wait for the PID.
   */
  pid_t pid = waitpid(tpid, &state, 0);
  if (pid < 0) {
    return lisp_make_number(errno);
  }
  /*
   * Return the result.
   */
  atom_t res = WIFEXITED(state) ?
    lisp_make_number(WEXITSTATUS(state)) :
    UP(NIL);
  return res;
}

LISP_PLUGIN_REGISTER(wait, wait, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
