#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

BINARY_NUMBER_GEN(sub, -, X, Y);
LISP_PLUGIN_REGISTER(sub, -, X, Y, NIL)
