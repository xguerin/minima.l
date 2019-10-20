#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

BINARY_NUMBER_GEN(div, /, X, Y);
LISP_PLUGIN_REGISTER(div, /, X, Y, NIL)
