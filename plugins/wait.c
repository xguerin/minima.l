#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

atom_t
lisp_function_wait(const atom_t closure, const atom_t cell)
{
  atom_t car = lisp_eval(closure, lisp_car(cell));
  int tpid = car->number, state;
  X(car); X(cell);
  pid_t pid = waitpid(tpid, &state, 0);
  if (pid < 0) {
    return lisp_make_number(errno);
  }
  atom_t res = WIFEXITED(state) ?
    lisp_make_number(WEXITSTATUS(state)) :
    UP(NIL);
  return res;
}

LISP_PLUGIN_REGISTER(wait, wait)
