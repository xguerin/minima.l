#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <lisp/utils.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

atom_t
lisp_function_load(const atom_t closure, const atom_t cell)
{
  /*
   * Get CAR/CDR.
   */
  atom_t car = lisp_eval(closure, lisp_car(cell));
  X(cell);
  /*
   * Construct the file name.
   */
  char buffer[PATH_MAX + 1];
  lisp_make_string(car, buffer, 0);
  X(car);
  /*
   * Open the file.
   */
  int fd = open(buffer, O_RDONLY);
  if (fd < 0) {
    return UP(NIL);
  }
  /*
   * Push the context.
   */
  PUSH_IO_CONTEXT(ICHAN, fd);
  /*
   * Load all the entries
   */
  atom_t input, res = UP(NIL);
  while ((input = lisp_read(NIL, UP(NIL))) != NIL) {
    X(res);
    res = lisp_eval(NIL, input);
  }
  X(input);
  /*
   * Pop the context and return the value.
   */
  POP_IO_CONTEXT(ICHAN);
  close(fd);
  return res;
}

LISP_REGISTER(load, load)
