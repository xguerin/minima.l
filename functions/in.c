#include "utils.h"
#include <fcntl.h>
#include <limits.h>
#include <lisp/lisp.h>
#include <lisp/slab.h>
#include <string.h>

static void
lisp_make_string(const atom_t cell, char * const buffer, const size_t idx)
{
  /*
   * Terminate the string.
   */
  if (IS_NULL(cell) || idx == PATH_MAX) {
    *buffer = '\0';
    return;
  }
  /*
   * Process the chars.
   */
  lisp_make_string(CDR(cell), buffer + 1, idx + 1);
  *buffer = CAR(cell)->number;
}

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
  char buffer[PATH_MAX + 1];
  lisp_make_string(car, buffer, 0);
  X(car);
  /*
   * Open the file.
   */
  TRACE("%s", buffer);
  int fd = open(buffer, O_RDONLY);
  if (fd < 0) {
    X(prg);
    return UP(NIL);
  }
  /*
   * Push the context.
   */
  PUSH_IO_CONTEXT(ICHAN, fd);
  atom_t res = lisp_eval(closure, prg);
  /*
   * Pop the context and return the value.
   */
  POP_IO_CONTEXT(ICHAN);
  return res;
}

LISP_REGISTER(in, in)
