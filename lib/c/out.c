#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

atom_t
lisp_function_out(const atom_t closure, const atom_t cell)
{
  int fd = 1;
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
      fd = open(buffer, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
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
   * Create the file handle.
   */
  FILE* handle = fdopen(fd, "a");
  if (handle == NULL) {
    X(prg);
    return UP(NIL);
  }
  /*
   * Push the context, eval the prog, pop the context.
   */
  PUSH_IO_CONTEXT(OCHAN, handle);
  atom_t res = lisp_prog(closure, prg, UP(NIL));
  POP_IO_CONTEXT(OCHAN);
  /*
   * Close the file handle.
   */
  fclose(handle);
  /*
   * Close the FD if necessary and return the value.
   */
  if (fd != 1) {
    close(fd);
  }
  return res;
}

LISP_PLUGIN_REGISTER(out, out)