#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

BINARY_NUMBER_GEN(add, +, X, Y);
LISP_PLUGIN_REGISTER(add, +, X, Y, NIL)
