#pragma once

#include <mnml/lisp.h>
#include <mnml/utils.h>
#include <stdbool.h>

/*
 * Plugin management.
 */

extern atom_t PLUGINS;

const char * lisp_prefix();

atom_t lisp_plugin_load(const atom_t sym);

void lisp_plugin_cleanup();

/*
 * Helpful macros.
 */

#define LISP_PLUGIN_REGISTER(__s, __n)                              \
                                                                    \
const char *                                                        \
lisp_plugin_name()                                                  \
{                                                                   \
  return #__n;                                                      \
}                                                                   \
                                                                    \
atom_t                                                              \
lisp_plugin_register()                                              \
{                                                                   \
  MAKE_SYMBOL_STATIC(inp, #__n, sizeof(#__n));                      \
  atom_t sym = lisp_make_symbol(inp);                               \
  atom_t val = lisp_make_number((uintptr_t)lisp_function_ ## __s);  \
  atom_t res = UP(val);                                             \
  GLOBALS = lisp_setq(GLOBALS, lisp_cons(sym, val));                \
  X(sym); X(val);                                                   \
  return res;                                                       \
}

#define PREDICATE_GEN(_n, _o)                                   \
static atom_t                                                   \
lisp_function_is ## _n(const atom_t closure, const atom_t cell) \
{                                                               \
  atom_t car = lisp_eval(closure, lisp_car(cell));              \
  atom_t cdr = lisp_cdr(cell);                                  \
  X(cell);                                                      \
  atom_t res = _o(car) ? TRUE : NIL;                            \
  atom_t cps = lisp_car(cdr);                                   \
  X(car); X(cdr);                                               \
  return lisp_rtrn(closure, UP(res), cps);                      \
}

#define BINARY_BOOLEAN_GEN(_n, _o)                            \
static atom_t                                                 \
lisp_function_ ## _n(const atom_t closure, const atom_t cell) \
{                                                             \
  atom_t vl0 = lisp_eval(closure, lisp_car(cell));            \
  atom_t cdr = lisp_cdr(cell);                                \
  X(cell);                                                    \
  atom_t vl1 = lisp_eval(closure, lisp_car(cdr));             \
  atom_t rem = lisp_cdr(cdr);                                 \
  X(cdr);                                                     \
  atom_t res = IS_TRUE(vl0) _o IS_TRUE(vl1) ? TRUE : NIL;     \
  atom_t cps = lisp_car(rem);                                 \
  X(vl0); X(vl1); X(rem);                                     \
  return lisp_rtrn(closure, UP(res), cps);                    \
}

#define BINARY_NUMBER_GEN(_n, _o)                             \
static atom_t                                                 \
lisp_function_ ## _n(const atom_t closure, const atom_t cell) \
{                                                             \
  atom_t vl0 = lisp_eval(closure, lisp_car(cell));            \
  atom_t cdr = lisp_cdr(cell);                                \
  X(cell);                                                    \
  atom_t vl1 = lisp_eval(closure, lisp_car(cdr));             \
  atom_t rem = lisp_cdr(cdr);                                 \
  X(cdr);                                                     \
  atom_t res = lisp_make_number(vl0->number _o vl1->number);  \
  atom_t cps = lisp_car(rem);                                 \
  X(vl0); X(vl1); X(rem);                                     \
  return lisp_rtrn(closure, res, cps);                        \
}

#define BINARY_COMPARE_GEN(_n, _o)                            \
static atom_t                                                 \
lisp_function_ ## _n(const atom_t closure, const atom_t cell) \
{                                                             \
  atom_t vl0 = lisp_eval(closure, lisp_car(cell));            \
  atom_t cdr = lisp_cdr(cell);                                \
  X(cell);                                                    \
  atom_t vl1 = lisp_eval(closure, lisp_car(cdr));             \
  atom_t rem = lisp_cdr(cdr);                                 \
  X(cdr);                                                     \
  atom_t res = vl0->number _o vl1->number ? TRUE : NIL;       \
  atom_t cps = lisp_car(rem);                                 \
  X(vl0); X(vl1); X(rem);                                     \
  return lisp_rtrn(closure, UP(res), cps);                    \
}
