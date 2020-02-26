#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

PREDICATE_GEN(str, lisp_is_string, X);
LISP_MODULE_SETUP(isstr, str?, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
