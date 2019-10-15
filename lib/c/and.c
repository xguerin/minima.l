#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

BINARY_BOOLEAN_GEN(and, &&);
LISP_PLUGIN_REGISTER(and, and)
