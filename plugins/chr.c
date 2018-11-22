#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <string.h>

atom_t
lisp_function_chr(const atom_t closure, const atom_t cell)
{
  atom_t car = lisp_eval(closure, lisp_car(cell));
  char val = car->number;
  X(car); X(cell);
  return lisp_make_char(val);
}

LISP_REGISTER(chr, chr)
