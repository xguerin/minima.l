#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

BINARY_NUMBER_GEN(mul, *, X, Y);
LISP_PLUGIN_REGISTER(mul, *, X, Y, NIL)
