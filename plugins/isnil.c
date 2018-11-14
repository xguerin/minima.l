#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

PREDICATE_GEN(nil, IS_NULL);
LISP_REGISTER(isnil, nil?)
