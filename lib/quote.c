#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_function_quote(const lisp_t lisp, const atom_t closure,
                    const atom_t arguments)
{
  LISP_LOOKUP(cell, arguments, @);
  return cell;
}

LISP_PLUGIN_REGISTER(quote, quote, @)

// vim: tw=80:sw=2:ts=2:sts=2:et
