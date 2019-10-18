#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

static atom_t
lisp_function_str(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(car, arguments, X);
  char buffer[17] = { 0 };
  strncpy(buffer, car->symbol.val, LISP_SYMBOL_LENGTH);
  X(car);
  return lisp_make_string(buffer, strlen(buffer));
}

LISP_PLUGIN_REGISTER(str, str, X)
