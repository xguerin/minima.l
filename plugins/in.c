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
  int fd = 0;
  char buffer[PATH_MAX + 1];
  size_t len = lisp_make_string(car, buffer, 0);
  X(car);
  /*
   * If the file name's length > 0, open the file with that name.
   */
  if (len > 0) {
    fd = open(buffer, O_RDONLY);
    if (fd < 0) {
      X(prg);
      return UP(NIL);
    }
  }
  /*
   * Push the context, eval the prog, pop the context.
   */
  PUSH_IO_CONTEXT(ICHAN, fd);
  atom_t res = lisp_eval(closure, prg);
  POP_IO_CONTEXT(ICHAN);
  /*
   * Close the FD if necessary and return the value.
   */
  if (len > 0) {
    close(fd);
  }
  return res;
}

LISP_REGISTER(in, in)
