#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

#define IS_LIST(__c) (IS_PAIR(__c) || IS_NULL(__c))

PREDICATE_GEN(lst, IS_LIST, X);
LISP_PLUGIN_REGISTER(islst, lst?, X)
