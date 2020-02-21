#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

PREDICATE_GEN(atm, IS_ATOM, X);
LISP_PLUGIN_REGISTER(isatm, atm?, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
