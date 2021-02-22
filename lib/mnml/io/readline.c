#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <stdio.h>

static atom_t USED
lisp_function_readline(const lisp_t lisp, UNUSED const atom_t closure)
{
  FILE* handle = (FILE*)CAR(CAR(lisp->ichan))->number;
  /*
   * Read a line.
   */
  char* line = NULL;
  size_t cap = 0;
  ssize_t len = getline(&line, &cap, handle);
  if (len < 0) {
    MAKE_SYMBOL_STATIC(eof_s, "EOF", 3);
    return lisp_make_symbol(lisp, eof_s);
  }
  /*
   * Remove the extra newline character.
   */
  if (len > 0 && line[len - 1] != 0) {
    line[len - 1] = 0;
    len -= 1;
  }
  /*
   * Produce the result.
   */
  atom_t res = lisp_make_string(lisp, line, len);
  free(line);
  return res;
}

LISP_MODULE_SETUP(readline, readline)

// vim: tw=80:sw=2:ts=2:sts=2:et
