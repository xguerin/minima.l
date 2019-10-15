#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

PREDICATE_GEN(atm, IS_ATOM);
LISP_PLUGIN_REGISTER(isatm, atm?)
