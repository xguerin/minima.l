#pragma once

#include <mnml/lisp.h>
#include <mnml/maker.h>
#include <mnml/utils.h>
#include <stdbool.h>

/*
 * Plugin management.
 */

extern atom_t PLUGINS;

atom_t lisp_plugin_load_immediate(const char * const sym, const char * const path);
atom_t lisp_plugin_load(const atom_t cell, const atom_t path);

void lisp_plugin_cleanup();

/*
 * Initialization macros.
 */

#define LISP_PLUGIN_REGISTER(__s, __n, ...)                     \
                                                                \
const char *                                                    \
lisp_plugin_name()                                              \
{                                                               \
  return #__n;                                                  \
}                                                               \
                                                                \
atom_t                                                          \
lisp_plugin_register()                                          \
{                                                               \
  MAKE_SYMBOL_STATIC(inp, #__n, LISP_GET_SYMBOL_LENGTH(#__n));  \
  atom_t sym = lisp_make_symbol(inp);                           \
  LISP_CONS(arg, ## __VA_ARGS__);                               \
  uintptr_t fun = (uintptr_t)lisp_function_ ## __s;             \
  atom_t adr = lisp_make_number(fun);                           \
  atom_t cn0 = lisp_cons(NIL, adr);                             \
  atom_t cn1 = lisp_cons(NIL, cn0);                             \
  atom_t val = lisp_cons(arg, cn1);                             \
  atom_t res = UP(val);                                         \
  atom_t cns = lisp_cons(sym, val);                             \
  TRACE_PLUG_SEXP(cns);                                         \
  GLOBALS = lisp_setq(GLOBALS, cns);                            \
  X(sym, adr, NIL, cn0, cn1, arg, val);                         \
  return res;                                                   \
}

/*
 * Closure lookup macro.
 */

#define LISP_LOOKUP(_v, _c, _x)                                 \
  MAKE_SYMBOL_STATIC(_##_v, #_x, LISP_GET_SYMBOL_LENGTH(#_x));  \
  atom_t _v = lisp_lookup_immediate(_c, _##_v)

/*
 * Plugin generators.
 */

#define PREDICATE_GEN(_n, _o, _x)                               \
static atom_t                                                   \
lisp_function_is ## _n(const atom_t closure, const atom_t args) \
{                                                               \
  LISP_LOOKUP(car, args, _x);                                   \
  atom_t res = _o(car) ? TRUE : NIL;                            \
  X(car);                                                       \
  return UP(res);                                               \
}

#define BINARY_BOOLEAN_GEN(_n, _o, _x, _y)                      \
static atom_t                                                   \
lisp_function_ ## _n(const atom_t closure, const atom_t args)   \
{                                                               \
  LISP_LOOKUP(vl0, args, _x);                                   \
  LISP_LOOKUP(vl1, args, _y);                                   \
  atom_t res = IS_TRUE(vl0) _o IS_TRUE(vl1) ? TRUE : NIL;       \
  X(vl0, vl1);                                                  \
  return UP(res);                                               \
}

#define BINARY_NUMBER_GEN(_n, _o, _x, _y)                       \
static atom_t                                                   \
lisp_function_ ## _n(const atom_t closure, const atom_t args)   \
{                                                               \
  LISP_LOOKUP(vl0, args, _x);                                   \
  LISP_LOOKUP(vl1, args, _y);                                   \
  atom_t res = lisp_make_number(vl0->number _o vl1->number);    \
  X(vl0, vl1);                                                  \
  return res;                                                   \
}

#define BINARY_COMPARE_GEN(_n, _o, _x, _y)                      \
static atom_t                                                   \
lisp_function_ ## _n(const atom_t closure, const atom_t args)   \
{                                                               \
  LISP_LOOKUP(vl0, args, _x);                                   \
  LISP_LOOKUP(vl1, args, _y);                                   \
  atom_t res = vl0->number _o vl1->number ? TRUE : NIL;         \
  X(vl0, vl1);                                                  \
  return UP(res);                                               \
}
