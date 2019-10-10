#include <cstring>

extern "C"
{
#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
}

/*
 * COMPILE entry point.
 */

static atom_t
lisp_function_compile(const atom_t closure, const atom_t cell)
{
  return UP(NIL);
}

extern "C" {
LISP_PLUGIN_REGISTER(compile, compile)
}
