#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

PREDICATE_GEN(tru, IS_TRUE, X);
LISP_MODULE_SETUP(istru, tru?, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
