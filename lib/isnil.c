#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

PREDICATE_GEN(nil, IS_NULL, X);
LISP_PLUGIN_REGISTER(isnil, nil?, X)

// vim: tw=80:sw=2:ts=2:sts=2:et
