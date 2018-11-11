#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <string.h>

PREDICATE_GEN(atm, IS_ATOM);
LISP_REGISTER(isatm, atm?)
