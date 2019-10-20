#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

PREDICATE_GEN(chr, IS_CHAR, X);
LISP_PLUGIN_REGISTER(ischr, chr?, X)
