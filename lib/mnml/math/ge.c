#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

BINARY_COMPARE_GEN(ge, >=, X, Y);
LISP_MODULE_SETUP(ge, >=, X, Y, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
