#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

PREDICATE_GEN(tru, IS_TRUE);
LISP_REGISTER(istru, tru?)
