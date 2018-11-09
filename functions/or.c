#include "utils.h"
#include <lisp/lisp.h>
#include <lisp/slab.h>
#include <string.h>

BINARY_BOOLEAN_GEN(or, ||);
LISP_REGISTER(or, ||)
