#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <string.h>

atom_t
lisp_function_str(const atom_t closure, const atom_t cell)
{
  char buffer[17] = { 0 };
  atom_t car = lisp_eval(closure, lisp_car(cell));
  strncpy(buffer, car->symbol.val, LISP_SYMBOL_LENGTH);
  X(car); X(cell);
  return lisp_make_string(buffer, strlen(buffer));
}

LISP_PLUGIN_REGISTER(str, str)
