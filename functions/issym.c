#include "utils.h"
#include <lisp/lisp.h>
#include <lisp/slab.h>
#include <string.h>

PREDICATE_GEN(sym, IS_SETQ);
LISP_REGISTER(issym, sym?)
