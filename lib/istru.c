#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

PREDICATE_GEN(tru, IS_TRUE, X);
LISP_PLUGIN_REGISTER(istru, tru?, X)
