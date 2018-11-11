#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <string.h>

PREDICATE_GEN(nil, IS_NULL);
LISP_REGISTER(isnil, nil?)
