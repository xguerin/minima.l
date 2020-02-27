#pragma once

#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/maker.h>
#include <mnml/utils.h>
#include <stdbool.h>

/*
 * Lifecycle management.
 */

bool lisp_module_init();
void lisp_module_fini();

/*
 * Module entry.
 */

typedef const char* (*module_name_t)();
typedef atom_t (*module_load_t)(const lisp_t lisp);

typedef struct _module_entry
{
  module_name_t name;
  module_load_t load;
} module_entry_t;

atom_t lisp_module_load(const lisp_t lisp, const atom_t cell);

/*
 * Initialization macros.
 */

#define LISP_MODULE_DECL(__s)                    \
  extern const char* lisp_module_##__s##_name(); \
  extern atom_t lisp_module_##__s##_load()

#define LISP_MODULE_REGISTER(__s)                      \
  {                                                    \
    lisp_module_##__s##_name, lisp_module_##__s##_load \
  }

#define LISP_MODULE_SETUP(__s, __n, ...)                  \
                                                          \
  const char* lisp_module_##__s##_name() { return #__n; } \
                                                          \
  atom_t lisp_module_##__s##_load(const lisp_t lisp)      \
  {                                                       \
    MAKE_SYMBOL_STATIC(inp, #__n, LISP_SYMBOL_LENGTH);    \
    atom_t sym = lisp_make_symbol(inp);                   \
    LISP_CONS(arg, ##__VA_ARGS__);                        \
    uintptr_t fun = (uintptr_t)lisp_function_##__s;       \
    atom_t adr = lisp_make_number(fun);                   \
    atom_t cn0 = lisp_cons(NIL, adr);                     \
    atom_t cn1 = lisp_cons(NIL, cn0);                     \
    atom_t val = lisp_cons(arg, cn1);                     \
    atom_t cns = lisp_cons(sym, val);                     \
    TRACE_MODL_SEXP(cns);                                 \
    lisp->GLOBALS = lisp_setq(lisp->GLOBALS, cns);        \
    X(adr, cn0, cn1, arg, val);                           \
    return sym;                                           \
  }

/*
 * Closure lookup macro.
 */

#define LISP_LOOKUP(_v, _c, _x)                       \
  MAKE_SYMBOL_STATIC(_##_v, #_x, LISP_SYMBOL_LENGTH); \
  atom_t _v = lisp_lookup_immediate(_c, _##_v)

/*
 * Module generators.
 */

#define PREDICATE_GEN(_n, _o, _x)                                    \
  static atom_t lisp_function_is##_n(const lisp_t l, const atom_t c, \
                                     const atom_t args)              \
  {                                                                  \
    LISP_LOOKUP(car, args, _x);                                      \
    atom_t res = _o(car) ? TRUE : NIL;                               \
    X(car);                                                          \
    return UP(res);                                                  \
  }

#define BINARY_BOOLEAN_GEN(_n, _o, _x, _y)                         \
  static atom_t lisp_function_##_n(const lisp_t l, const atom_t c, \
                                   const atom_t args)              \
  {                                                                \
    LISP_LOOKUP(vl0, args, _x);                                    \
    LISP_LOOKUP(vl1, args, _y);                                    \
    atom_t res = (!IS_NULL(vl0))_o(!IS_NULL(vl1)) ? TRUE : NIL;    \
    X(vl0, vl1);                                                   \
    return UP(res);                                                \
  }

#define BINARY_NUMBER_GEN(_n, _o, _x, _y)                          \
  static atom_t lisp_function_##_n(const lisp_t l, const atom_t c, \
                                   const atom_t args)              \
  {                                                                \
    LISP_LOOKUP(vl0, args, _x);                                    \
    LISP_LOOKUP(vl1, args, _y);                                    \
    atom_t res = lisp_make_number(vl0->number _o vl1->number);     \
    X(vl0, vl1);                                                   \
    return res;                                                    \
  }

#define BINARY_COMPARE_GEN(_n, _o, _x, _y)                         \
  static atom_t lisp_function_##_n(const lisp_t l, const atom_t c, \
                                   const atom_t args)              \
  {                                                                \
    LISP_LOOKUP(vl0, args, _x);                                    \
    LISP_LOOKUP(vl1, args, _y);                                    \
    atom_t res = vl0->number _o vl1->number ? TRUE : NIL;          \
    X(vl0, vl1);                                                   \
    return UP(res);                                                \
  }

// vim: tw=80:sw=2:ts=2:sts=2:et