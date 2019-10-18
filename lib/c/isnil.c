#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

PREDICATE_GEN(nil, IS_NULL, X);
LISP_PLUGIN_REGISTER(isnil, nil?, X)
