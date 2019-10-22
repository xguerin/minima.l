#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

PREDICATE_GEN(tru, IS_TRUE, X);
LISP_PLUGIN_REGISTER(istru, tru?, X)

// vim: tw=80:sw=2:ts=2:sts=2:et
