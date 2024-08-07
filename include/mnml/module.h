#pragma once

#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/utils.h>
#include <stdbool.h>

/*
 * Lifecycle management.
 */

bool module_init(const lisp_t lisp);
void module_fini(const lisp_t lisp);

/*
 * Module entry.
 */

typedef const char* (*module_name_t)();
typedef atom_t (*module_load_t)(const lisp_t lisp);

typedef struct module_entry
{
  module_name_t name;
  module_load_t load;
} module_entry_t;

atom_t module_load(const lisp_t lisp, const atom_t cell);

void module_load_defaults(const lisp_t lisp);

/*
 * CONS helpers.
 */

#define S_0(__l, __r) atom_t __r = lisp_make_nil(__l)

#define S_1(__l, __r, _1)                    \
  S_0(__l, __r);                             \
  {                                          \
    MAKE_SYMBOL_STATIC(s_1, #_1);            \
    atom_t r_1 = lisp_make_symbol(__l, s_1); \
    X((__l), __r);                           \
    (__r) = r_1;                             \
  }

#define S_NIL(__l, __r, _1)                  \
  S_0(__l, __r);                             \
  {                                          \
    MAKE_SYMBOL_STATIC(s_1, #_1);            \
    atom_t __1 = lisp_make_symbol(__l, s_1); \
    (__r) = lisp_cons(__l, __1, __r);        \
  }

#define S_REM(__l, __r, _1)                  \
  S_0(__l, __r);                             \
  {                                          \
    X((__l), __r);                           \
    MAKE_SYMBOL_STATIC(s_1, #_1);            \
    atom_t __1 = lisp_make_symbol(__l, s_1); \
    MAKE_SYMBOL_STATIC(s_0, "REM");          \
    atom_t __0 = lisp_make_symbol(__l, s_0); \
    (__r) = lisp_cons(__l, __1, __0);        \
  }

#define S_2(__l, __r, _2, _1) S_##_1(__l, __r, _2)

#define S_3(__l, __r, _3, ...)               \
  S_2(__l, __r, __VA_ARGS__);                \
  {                                          \
    MAKE_SYMBOL_STATIC(s_3, #_3);            \
    atom_t __3 = lisp_make_symbol(__l, s_3); \
    __r = lisp_cons(__l, __3, __r);          \
  }

#define S_4(__l, __r, _4, ...)               \
  S_3(__l, __r, __VA_ARGS__);                \
  {                                          \
    MAKE_SYMBOL_STATIC(s_4, #_4);            \
    atom_t __4 = lisp_make_symbol(__l, s_4); \
    __r = lisp_cons(__l, __4, __r);          \
  }

#define S_5(__l, __r, _5, ...)               \
  S_4(__l, __r, __VA_ARGS__);                \
  {                                          \
    MAKE_SYMBOL_STATIC(s_5, #_5);            \
    atom_t __5 = lisp_make_symbol(__l, s_5); \
    __r = lisp_cons(__l, __5, __r);          \
  }

#define S_(_1, _2, _3, _4, _5, NAME, ...) NAME

#define LISP_CONS(__l, __r, ...)           \
  S_(__VA_ARGS__, S_5, S_4, S_3, S_2, S_1) \
  (__l, __r, __VA_ARGS__)

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

#define LISP_MODULE_SETUP(__s, __n, ...)                    \
                                                            \
  const char* USED lisp_module_##__s##_name()               \
  {                                                         \
    return #__n;                                            \
  }                                                         \
                                                            \
  atom_t USED lisp_module_##__s##_load(const lisp_t lisp)   \
  {                                                         \
    MAKE_SYMBOL_STATIC(inp, #__n);                          \
    atom_t sym = lisp_make_symbol(lisp, inp);               \
    LISP_CONS(lisp, arg, ##__VA_ARGS__);                    \
    uintptr_t fun = (uintptr_t)lisp_function_##__s;         \
    atom_t adr = lisp_make_number(lisp, fun);               \
    atom_t cn0 = lisp_cons(lisp, lisp_make_nil(lisp), adr); \
    atom_t val = lisp_cons(lisp, arg, cn0);                 \
    atom_t cns = lisp_cons(lisp, UP(sym), val);             \
    lisp->globals = lisp_setq(lisp, lisp->globals, cns);    \
    return sym;                                             \
  }

/*
 * Argument lookup macros.
 */

#ifdef LISP_ENABLE_DEBUG

#define LISP_ASSERT_ARG(_c, _a)                                            \
  {                                                                        \
    MAKE_SYMBOL_STATIC(sym_##_a, #_a);                                     \
    if (!lisp_symbol_match(CAR(CAR(_c)), sym_##_a)) {                      \
      ERROR("Argument mismatch: %.16s %s", CAR(CAR(_c))->symbol.val, #_a); \
      abort();                                                             \
    }                                                                      \
  }

#else

#define LISP_ASSERT_ARG(_c, _a)

#endif

#define A_0(_c, _p, ...) atom_t _p = _c

#define A_1(_c, _p, _1, ...) \
  A_0(_c, _p, __VA_ARGS__);  \
  LISP_ASSERT_ARG(_p, _1);   \
  atom_t _1 = CDR(CAR(_p));  \
  _p = CDR(_p)

#define A_2(_c, _p, _2, ...) \
  A_1(_c, _p, __VA_ARGS__);  \
  LISP_ASSERT_ARG(_p, _2);   \
  atom_t _2 = CDR(CAR(_p));  \
  _p = CDR(_p)

#define A_3(_c, _p, _3, ...) \
  A_2(_c, _p, __VA_ARGS__);  \
  LISP_ASSERT_ARG(_p, _3);   \
  atom_t _3 = CDR(CAR(_p));  \
  _p = CDR(_p)

#define A_4(_c, _p, _4, ...) \
  A_3(_c, _p, __VA_ARGS__);  \
  LISP_ASSERT_ARG(_p, _4);   \
  atom_t _4 = CDR(CAR(_p));  \
  _p = CDR(_p)

#define LISP_ARGS_(_1, _2, _3, _4, NAME, ...) NAME
#define LISP_ARGS(_c, _p, ...) \
  LISP_ARGS_(__VA_ARGS__, A_4, A_3, A_2, A_1)(_c, _p, __VA_ARGS__)

/*
 * Module generators.
 */

#define PREDICATE_GEN(_n, _o, _x)                                    \
  static atom_t lisp_function_is##_n(const lisp_t l, const atom_t c) \
  {                                                                  \
    LISP_ARGS(c, C, _x);                                             \
    return _o(_x) ? lisp_make_true(l) : lisp_make_nil(l);            \
  }

#define BINARY_BOOLEAN_GEN(_n, _o, _x, _y)                         \
  static atom_t lisp_function_##_n(const lisp_t l, const atom_t c) \
  {                                                                \
    LISP_ARGS(c, C, _x, _y);                                       \
    return (!IS_NULL(_x))_o(!IS_NULL(_y)) ? lisp_make_true(l)      \
                                          : lisp_make_nil(l);      \
  }

#define BINARY_NUMBER_GEN(_n, _o, _x, _y)                          \
  static atom_t lisp_function_##_n(const lisp_t l, const atom_t c) \
  {                                                                \
    LISP_ARGS(c, C, _x, _y);                                       \
    return lisp_make_number(l, (_x)->number _o _y->number);        \
  }

#define BINARY_COMPARE_GEN(_n, _o, _x, _y)                                    \
  static atom_t lisp_function_##_n(const lisp_t l, const atom_t c)            \
  {                                                                           \
    LISP_ARGS(c, C, _x, _y);                                                  \
    return (_x)->number _o _y->number ? lisp_make_true(l) : lisp_make_nil(l); \
  }

// vim: tw=80:sw=2:ts=2:sts=2:et
