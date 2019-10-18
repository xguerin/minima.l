#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

PREDICATE_GEN(atm, IS_ATOM, X);
LISP_PLUGIN_REGISTER(isatm, atm?, X)
