#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <string.h>

BINARY_BOOLEAN_GEN(and, &&);
LISP_REGISTER(and, &&)
