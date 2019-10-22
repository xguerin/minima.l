#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>

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
    res = lisp_plugin_load(car, NIL);
    X(car);
  }
  /*
   * Construct the file name.
   */
  else {
    /*
     * Construct the file name.
     */
    char buffer[PATH_MAX + 1];
    lisp_make_cstring(car, buffer, PATH_MAX, 0);
    res = lisp_load_file(buffer);
    X(car);
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

// vim: tw=80:sw=2:ts=2:sts=2:et
