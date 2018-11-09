#include "utils.h"
#include <lisp/lisp.h>
#include <lisp/slab.h>
#include <string.h>

PREDICATE_GEN(sym, IS_SYMB);
LISP_REGISTER(issym, sym?)
