#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

BINARY_COMPARE_GEN(le, <=, X, Y);
LISP_PLUGIN_REGISTER(le, <=, X, Y, NIL)
