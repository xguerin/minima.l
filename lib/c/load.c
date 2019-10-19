#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>

static const char *
lisp_expand_path(const char * path)
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

static atom_t
lisp_load_plugin(const atom_t symb, const atom_t path)
{
  atom_t result = lisp_plugin_load(symb, path);
  X(symb); X(path);
  return result;
}

static atom_t
lisp_load_file(const atom_t cell)
{
  /*
   * Construct the file name.
   */
  char buffer[PATH_MAX + 1];
  lisp_make_cstring(cell, buffer, PATH_MAX, 0);
  X(cell);
  /*
   * Expand the path.
   */
  const char * path = lisp_expand_path(buffer);
  /*
   * Open the file.
   */
  FILE* handle = fopen(path, "r");
  if (handle == NULL) {
    return UP(NIL);
  }
  /*
   * Push the context.
   */
  PUSH_IO_CONTEXT(ICHAN, handle);
  /*
   * Load all the entries
   */
  atom_t input, res = UP(NIL);
  while ((input = lisp_read(NIL, UP(NIL))) != NULL) {
    X(res);
    res = lisp_eval(NIL, input);
  }
  /*
   * Pop the context and return the value.
   */
  POP_IO_CONTEXT(ICHAN);
  fclose(handle);
  return res;
}

static atom_t
lisp_load(const atom_t closure, const atom_t cell)
{
  atom_t res;
  TRACE_SEXP(cell);
  /*
   * Get CAR/CDR.
   */
  atom_t car = lisp_eval(closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  /*
   * If it's a symbol, load it from plugins.
   */
  if (IS_SYMB(car)) {
    res = lisp_load_plugin(car, UP(NIL));
  }
  /*
   * Construct the file name.
   */
  else {
    res = lisp_load_file(car);
  }
  /*
   * If CDR is NIL, return the result.
   */
  if (IS_NULL(cdr)) {
    X(cdr);
    return res;
  }
  /*
   * Return the next evaluation otherwise.
   */
  X(res);
  return lisp_load(closure, cdr);
}

static atom_t
lisp_function_load(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(cell, arguments, @);
  return lisp_load(closure, cell);
}

LISP_PLUGIN_REGISTER(load, load, @)
