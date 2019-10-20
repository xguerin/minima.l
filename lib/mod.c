#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

BINARY_NUMBER_GEN(mod, %, X, Y);
LISP_PLUGIN_REGISTER(mod, %, X, Y, NIL)
