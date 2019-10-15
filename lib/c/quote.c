#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

atom_t
lisp_function_quote(const atom_t closure, const atom_t cell)
{
  return cell;
}

LISP_PLUGIN_REGISTER(quote, quote)
