#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

PREDICATE_GEN(num, IS_NUMB, X);
LISP_PLUGIN_REGISTER(isnum, num?, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
