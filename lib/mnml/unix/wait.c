#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <errno.h>
#include <sys/wait.h>

static atom_t USED
lisp_function_wait(UNUSED const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, X);
  /*
   * Get the PID.
   */
  int tpid = X->number, state;
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
  atom_t res =
    WIFEXITED(state) ? lisp_make_number(WEXITSTATUS(state)) : lisp_make_nil();
  return res;
}

LISP_MODULE_SETUP(wait, wait, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
