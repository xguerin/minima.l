#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <lisp/utils.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

atom_t
lisp_function_out(const atom_t closure, const atom_t cell)
{
  /*
   * Get CAR/CDR.
   */
  atom_t car = lisp_eval(closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  atom_t prg = lisp_car(cdr);
  X(cdr);
  /*
   * Construct the file name.
   */
  char buffer[PATH_MAX + 1];
  lisp_make_string(car, buffer, 0);
  X(car);
  /*
   * Open the file.
   */
  int fd = open(buffer, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    X(prg);
    return UP(NIL);
  }
  /*
   * Push the context.
   */
  PUSH_IO_CONTEXT(OCHAN, fd);
  atom_t res = lisp_eval(closure, prg);
  /*
   * Pop the context and return the value.
   */
  POP_IO_CONTEXT(OCHAN);
  close(fd);
  return res;
}

LISP_REGISTER(out, out)
