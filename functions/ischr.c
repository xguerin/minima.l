#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <string.h>

PREDICATE_GEN(chr, IS_CHAR);
LISP_REGISTER(ischr, chr?)
