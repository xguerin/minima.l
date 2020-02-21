#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

BINARY_COMPARE_GEN(ge, >=, X, Y);
LISP_PLUGIN_REGISTER(ge, >=, X, Y, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
