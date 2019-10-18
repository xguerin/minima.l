#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

BINARY_COMPARE_GEN(ge, >=, X, Y);
LISP_PLUGIN_REGISTER(ge, >=, X, Y, NIL)
