#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <string.h>

PREDICATE_GEN(num, IS_NUMB);
LISP_REGISTER(isnum, num?)
