#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

BINARY_BOOLEAN_GEN(or, ||, X, Y);
LISP_PLUGIN_REGISTER(or, or, X, Y, NIL)
