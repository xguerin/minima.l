#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

#define IS_LIST(__c) (IS_PAIR(__c) || IS_NULL(__c))

PREDICATE_GEN(lst, IS_LIST);
LISP_PLUGIN_REGISTER(islst, lst?)
