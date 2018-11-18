#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LEN 1024

static atom_t
lisp_readlines_append(const atom_t cell, const char * const buffer,
                      const size_t len)
{
  atom_t str = lisp_make_string(buffer, len);
  atom_t con = lisp_cons(str, NIL);
  atom_t res = lisp_conc(cell, con);
  X(con); X(str); X(cell);
  return res;
}

static atom_t
lisp_readlines(const atom_t cell, char * const buffer, size_t * const offset)
{
  atom_t res = cell;
  char * p = buffer, * n = NULL;
  size_t len = 0;
  /*
   * Invoke strstr() to find the next occurance of CR.
   */
  do {
    n = strstr(p, "\n");
    if (n != NULL) {
      len = n - p;
      res = lisp_readlines_append(res, p, len);
      p = n + 1;
    }
  }
  while (n != NULL);
  /*
   * If there is a remainder, copy it at the beginning of the buffer.
   */
  if (*p != 0) {
    strcpy(buffer, p);
    *offset = strlen(buffer);
  }
  /*
   */
  return res;
}

atom_t
lisp_function_readlines(const atom_t closure, const atom_t cell)
{
  atom_t car = lisp_eval(closure, lisp_car(cell));
  int fd = car->number;
  X(car); X(cell);
  /*
   */
  atom_t res = UP(NIL);
  char buffer[BUFFER_LEN + 1];
  ssize_t len = 0;
  size_t off = 0;
  /*
   * Read some data and process any lines present.
   */
  do {
    len = read(fd, buffer, BUFFER_LEN - off);
    buffer[off + len] = 0;
    if (len > 0) {
      res = lisp_readlines(res, buffer, &off);
    }
  }
  while (len > 0);
  /*
   * Process any remaining data in the buffer.
   */
  return lisp_readlines(res, buffer, &off);
}

LISP_REGISTER(readlines, readlines)
