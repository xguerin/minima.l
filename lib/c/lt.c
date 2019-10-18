#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

BINARY_COMPARE_GEN(lt, <, X, Y);
LISP_PLUGIN_REGISTER(lt, <, X, Y)
