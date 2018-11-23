#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

PREDICATE_GEN(num, IS_NUMB);
LISP_PLUGIN_REGISTER(isnum, num?)
