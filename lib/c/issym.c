#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

PREDICATE_GEN(sym, IS_SYMB);
LISP_PLUGIN_REGISTER(issym, sym?)
