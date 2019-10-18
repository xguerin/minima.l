#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t
lisp_function_chr(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(car, arguments, X);
  char val = car->number;
  X(car);
  return lisp_make_char(val);
}

LISP_PLUGIN_REGISTER(chr, chr, X)
