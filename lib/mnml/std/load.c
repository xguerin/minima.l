#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>

static atom_t
lisp_load(const lisp_t lisp, const atom_t closure, const atom_t cell)
{
  atom_t res;
  /*
   * Get CAR/CDR.
   */
  atom_t car = lisp_eval(lisp, closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  /*
   * Make sure that we are dealing with a list.
   */
  if (!IS_PAIR(car)) {
    X(car, cdr);
    return UP(NIL);
  }
  /*
   * If it's a string, load it as a script.
   */
  if (lisp_is_string(car)) {
    /*
     * Construct the file name.
     */
    char buffer[PATH_MAX + 1];
    lisp_make_cstring(car, buffer, PATH_MAX, 0);
    res = lisp_load_file(lisp, buffer);
    X(car);
  }
  /*
   * Otherwise load the binary module.
   */
  else {
    res = module_load(lisp, car);
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
  return lisp_load(lisp, closure, cdr);
}

static atom_t
lisp_function_load(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, cell, closure, @);
  return lisp_load(lisp, closure, cell);
}

LISP_MODULE_SETUP(load, load, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
