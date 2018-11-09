#pragma once

#define LISP_REGISTER(__s, __n)                                     \
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
  union { uint64_t tag; char symbol[8]; } u = { 0 };                \
  strcpy(u.symbol, #__n);                                           \
  atom_t sym = lisp_make_inline(u.tag);                             \
  atom_t val = lisp_make_number((uintptr_t)lisp_function_ ## __s);  \
  atom_t res = UP(val);                                             \
  GLOBALS = lisp_setq(GLOBALS, sym, val);                           \
  return res;                                                       \
}

#define PREDICATE_GEN(_n, _o)                                   \
static atom_t                                                   \
lisp_function_is ## _n(const atom_t closure, const atom_t cell) \
{                                                               \
  atom_t car = lisp_eval(closure, lisp_car(cell));              \
  X(cell);                                                      \
  atom_t res = _o(car) ? TRUE : NIL;                            \
  X(car);                                                       \
  return UP(res);                                               \
}

#define BINARY_BOOLEAN_GEN(_n, _o)                            \
static atom_t                                                 \
lisp_function_ ## _n(const atom_t closure, const atom_t cell) \
{                                                             \
  atom_t vl0 = lisp_eval(closure, lisp_car(cell));            \
  atom_t cdr = lisp_cdr(cell);                                \
  X(cell);                                                    \
  atom_t vl1 = lisp_eval(closure, lisp_car(cdr));             \
  X(cdr);                                                     \
  atom_t res = IS_TRUE(vl0) _o IS_TRUE(vl1) ? TRUE : NIL;     \
  X(vl0); X(vl1);                                             \
  return UP(res);                                             \
}

#define BINARY_NUMBER_GEN(_n, _o)                             \
static atom_t                                                 \
lisp_function_ ## _n(const atom_t closure, const atom_t cell) \
{                                                             \
  atom_t vl0 = lisp_eval(closure, lisp_car(cell));            \
  atom_t cdr = lisp_cdr(cell);                                \
  X(cell);                                                    \
  atom_t vl1 = lisp_eval(closure, lisp_car(cdr));             \
  X(cdr);                                                     \
  atom_t res = lisp_make_number(vl0->number _o vl1->number);  \
  X(vl0); X(vl1);                                             \
  return res;                                                 \
}

#define BINARY_COMPARE_GEN(_n, _o)                              \
static atom_t                                                   \
lisp_function_ ## _n(const atom_t closure, const atom_t cell)   \
{                                                               \
  atom_t vl0 = lisp_eval(closure, lisp_car(cell));              \
  atom_t cdr = lisp_cdr(cell);                                  \
  X(cell);                                                      \
  atom_t vl1 = lisp_eval(closure, lisp_car(cdr));               \
  X(cdr);                                                       \
  atom_t res = vl0->number _o vl1->number ? TRUE : NIL;         \
  X(vl0); X(vl1);                                               \
  return UP(res);                                               \
}
