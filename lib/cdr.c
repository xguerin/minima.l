#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_function_cdr(const lisp_t lisp, const atom_t closure,
                  const atom_t arguments)
{
  LISP_LOOKUP(arg, arguments, X);
  atom_t res = lisp_cdr(arg);
  X(arg);
  return res;
}

LISP_PLUGIN_REGISTER(cdr, cdr, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
