#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t USED
lisp_function_quote(UNUSED const lisp_t lisp, const atom_t closure)
{
  TRACE_CLOS_SEXP(closure);
  LISP_ARGS(closure, C, ANY);
  return UP(ANY);
}

LISP_MODULE_SETUP(quote, quote, ANY)

// vim: tw=80:sw=2:ts=2:sts=2:et
