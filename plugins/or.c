#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>

BINARY_BOOLEAN_GEN(or, ||);
LISP_REGISTER(or, or)
