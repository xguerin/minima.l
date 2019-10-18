#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

PREDICATE_GEN(num, IS_NUMB, X);
LISP_PLUGIN_REGISTER(isnum, num?, X)
