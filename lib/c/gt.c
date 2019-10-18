#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

BINARY_COMPARE_GEN(gt, >, X, Y);
LISP_PLUGIN_REGISTER(gt, >, X, Y, NIL)
