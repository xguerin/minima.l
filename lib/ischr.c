#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

PREDICATE_GEN(chr, IS_CHAR, X);
LISP_PLUGIN_REGISTER(ischr, chr?, X)

// vim: tw=80:sw=2:ts=2:sts=2:et
