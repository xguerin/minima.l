#include <mnml/debug.h>
#include <mnml/maker.h>
#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <fcntl.h>
#include <limits.h>

static atom_t USED
lisp_load(const lisp_t lisp, const atom_t closure, const atom_t cell)
{
  atom_t res;
  /*
   * Get CAR/CDR.
   */
  atom_t car = lisp_eval(lisp, closure, lisp_car(lisp, cell));
  atom_t cdr = lisp_cdr(lisp, cell);
  X(lisp->slab, cell);
  /*
   * Make sure that we are dealing with a symbol or a list.
   */
  switch (car->type) {
    /*
     * If it's a scoped symbol, load the one symbol from the binary module
     * (shortcut for '(MOD . SYM)).
     */
    case T_SCOPED_SYMBOL: {
      atom_t nsp = lisp_scope_get_name(lisp, car);
      atom_t sym = lisp_scope_get_symb(lisp, car);
      X(lisp->slab, car);
      atom_t lst = lisp_cons(lisp, sym, lisp_make_nil(lisp));
      atom_t mod = lisp_cons(lisp, nsp, lst);
      res = module_load(lisp, mod);
      break;
    }
    /*
     * If it's a symbol, load the binary module (shortcut for '(MOD . T)).
     */
    case T_SYMBOL: {
      atom_t mod = lisp_cons(lisp, car, lisp_make_true(lisp));
      res = module_load(lisp, mod);
      break;
    }
    /*
     * If it's a pair, it can be either a string or a binary module.
     */
    case T_PAIR: {
      /*
       * If it's a string, load it as a script.
       */
      if (lisp_is_string(car)) {
        /*
         * Construct the file name.
         */
        char buffer[PATH_MAX + 1];
        lisp_make_cstring(car, buffer, PATH_MAX, 0);
        X(lisp->slab, car);
        /*
         * Load the file.
         */
        res = lisp_load_file(lisp, buffer);
      }
      /*
       * Otherwise load the binary module.
       */
      else {
        res = module_load(lisp, car);
      }
      break;
    }
    default: {
      X(lisp->slab, car);
      res = lisp_make_nil(lisp);
    }
  }
  /*
   * If CDR is NIL, return the result.
   */
  if (IS_NULL(cdr)) {
    X(lisp->slab, cdr);
    return res;
  }
  /*
   * Return the next evaluation otherwise.
   */
  X(lisp->slab, res);
  return lisp_load(lisp, closure, cdr);
}

static atom_t
lisp_function_load(const lisp_t lisp, const atom_t closure)
{
  TRACE_CLOS_SEXP(closure);
  LISP_ARGS(closure, C, ANY);
  return lisp_load(lisp, C, UP(ANY));
}

LISP_MODULE_SETUP(load, load, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
