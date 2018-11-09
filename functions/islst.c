#include "utils.h"
#include <lisp/lisp.h>
#include <lisp/slab.h>
#include <string.h>

PREDICATE_GEN(lst, IS_PAIR);
LISP_REGISTER(islst, lst?)
