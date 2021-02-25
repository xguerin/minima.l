#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

#define IS_ANY_SYMB(__x) (IS_SCOP(__x) || IS_SYMB(__x))

PREDICATE_GEN(sym, IS_ANY_SYMB, X);
LISP_MODULE_SETUP(issym, sym?, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
