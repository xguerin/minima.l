#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

PREDICATE_GEN(sym, IS_SYMB, X);
LISP_MODULE_SETUP(issym, sym?, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
