#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <errno.h>
#include <unistd.h>

#define BUFFER_LEN 1024

static atom_t
lisp_function_readlines(const atom_t closure, const atom_t arguments)
{
  FILE* handle = (FILE*)CAR(CAR(ICHAN))->number;
  /*
   */
  atom_t res = UP(NIL);
  char * buffer = malloc(BUFFER_LEN), *p;
  /*
   * Read some data and process any lines present.
   */
  do {
    p = fgets(buffer, BUFFER_LEN, handle);
    if (p == NULL) {
      continue;
    }
    size_t len = strlen(p);
    if (p[len - 1] == '\n') {
      p[len - 1] = '\0';
      len -= 1;
    }
    atom_t str = lisp_make_string(buffer, strlen(p));
    res = lisp_append(res, str);
  }
  while (p != NULL);
  /*
   * Clean-up and return the result.
   */
  free(buffer);
  return res;
}

LISP_PLUGIN_REGISTER(readlines, readlines)
