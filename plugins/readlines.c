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
lisp_readlines(const atom_t cell, char * const buffer, ssize_t * const rem)
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
  *rem = 0;
  if (*p != 0) {
    strcpy(buffer, p);
    *rem = strlen(buffer);
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
  size_t buflen = BUFFER_LEN;
  char * buffer = malloc(buflen + 1);
  ssize_t dlt, len = 0;
  /*
   * Read some data and process any lines present.
   */
  do {
    dlt = read(fd, buffer + len, buflen - len);
    len += dlt;
    buffer[len] = 0;
    if (strstr(buffer, "\n") == NULL) {
      buflen += buflen;
      buffer = realloc(buffer, buflen + 1);
      continue;
    }
    res = lisp_readlines(res, buffer, &len);
  }
  while (dlt > 0);
  /*
   * Process any remaining data in the buffer.
   */
  if (len > 0) {
    res = lisp_readlines_append(res, buffer, len);
  }
  free(buffer);
  return res;
}

LISP_REGISTER(readlines, readlines)
