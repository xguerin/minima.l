#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

#define IS_LIST(__c) (IS_PAIR(__c) || IS_NULL(__c))

PREDICATE_GEN(lst, IS_LIST, X);
LISP_PLUGIN_REGISTER(islst, lst?, X)

// vim: tw=80:sw=2:ts=2:sts=2:et
