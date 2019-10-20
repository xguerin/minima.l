#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

BINARY_BOOLEAN_GEN(and, &&, X, Y);
LISP_PLUGIN_REGISTER(and, and, X, Y, NIL)
