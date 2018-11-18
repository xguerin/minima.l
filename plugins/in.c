#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

atom_t
lisp_function_in(const atom_t closure, const atom_t cell)
{
  int fd = 0;
  char buffer[PATH_MAX + 1];
  /*
   * Get CAR/CDR.
   */
  atom_t car = lisp_eval(closure, lisp_car(cell));
  atom_t prg = lisp_cdr(cell);
  X(cell);
  /*
   * Process the CAR.
   */
  switch (car->type) {
    case T_NIL:
      X(car);
      break;
    case T_NUMBER:
      fd = car->number;
      X(car);
      break;
    case T_PAIR: {
      lisp_make_cstring(car, buffer, PATH_MAX, 0);
      X(car);
      fd = open(buffer, O_RDONLY);
      if (fd < 0) {
        X(prg);
        return UP(NIL);
      }
      break;
    }
    default:
      X(car); X(prg);
      return UP(NIL);
  }
  /*
   * Push the context, eval the prog, pop the context.
   */
  PUSH_IO_CONTEXT(ICHAN, fd);
  atom_t res = lisp_prog(closure, prg, UP(NIL));
  POP_IO_CONTEXT(ICHAN);
  /*
   * Close the FD if necessary and return the value.
   */
  if (fd > 0) {
    close(fd);
  }
  return res;
}

LISP_REGISTER(in, in)
