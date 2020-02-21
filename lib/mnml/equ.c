#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t
lisp_function_equ(const lisp_t lisp, const atom_t closure,
                  const atom_t arguments)
{
  LISP_LOOKUP(vl0, arguments, X);
  LISP_LOOKUP(vl1, arguments, Y);
  atom_t res = lisp_equ(vl0, vl1) ? TRUE : NIL;
  X(vl0, vl1);
  return UP(res);
}

LISP_PLUGIN_REGISTER(equ, =, X, Y, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
