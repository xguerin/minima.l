#include <lisp/lisp.h>
#include <lisp/plugin.h>
#include <lisp/slab.h>
#include <string.h>

#define IS_LIST(__c) (IS_PAIR(__c) || IS_NULL(__c))

PREDICATE_GEN(lst, IS_LIST);
LISP_REGISTER(islst, lst?)
