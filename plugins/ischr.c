#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

PREDICATE_GEN(chr, IS_CHAR);
LISP_PLUGIN_REGISTER(ischr, chr?)
