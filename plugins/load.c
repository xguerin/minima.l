#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

static const char * lisp_expand_path(const char * path)
{
  static char buffer[PATH_MAX];
  if (strncmp(path, "@lib", 4) == 0) {
    memset(buffer, 0, PATH_MAX);
    strcpy(buffer, lisp_prefix());
    strcat(buffer, "/lisp");
    strcat(buffer, &path[4]);
    return buffer;
  }
  return path;
}

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
   * Expand the path.
   */
  const char * path = lisp_expand_path(buffer);
  /*
   * Open the file.
   */
  int fd = open(path, O_RDONLY);
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
