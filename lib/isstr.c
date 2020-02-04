#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

PREDICATE_GEN(str, lisp_is_string, X);
LISP_PLUGIN_REGISTER(isstr, str?, X, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
