#include "functions.h"
#include "slab.h"
#include <string.h>

/*
 * QUOTE.
 */

atom_t
lisp_function_quote(const atom_t closure, const atom_t cell)
{
  return cell;
}

/*
 * EVAL.
 */

static atom_t
lisp_function_eval(const atom_t closure, const atom_t cell)
{
  atom_t car = lisp_eval(closure, lisp_car(cell));
  LISP_FREE(cell);
  return lisp_eval(closure, car);
}

static atom_t
lisp_function_prog(const atom_t closure, const atom_t cell)
{
  return lisp_prog(closure, cell, UP(NIL));
}

/*
 * CAR/CDR.
 */

static atom_t
lisp_function_car(const atom_t closure, const atom_t cell)
{
  atom_t arg = lisp_eval(closure, lisp_car(cell));
  atom_t res = lisp_car(arg);
  LISP_FREE(arg, cell);
  return res;
}

static atom_t
lisp_function_cdr(const atom_t closure, const atom_t cell)
{
  atom_t arg = lisp_eval(closure, lisp_car(cell));
  atom_t res = lisp_cdr(arg);
  LISP_FREE(arg, cell);
  return res;
}

/*
 * CONS/CONC.
 */

static atom_t
lisp_function_conc(const atom_t closure, const atom_t cell)
{
  atom_t fst = lisp_eval(closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  atom_t snd = lisp_eval(closure, lisp_car(cdr));
  atom_t res = lisp_conc(fst, snd);
  LISP_FREE(snd, cdr, cell);
  return res;
}

static atom_t
lisp_function_cons(const atom_t closure, const atom_t cell)
{
  atom_t fst = lisp_eval(closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  atom_t snd = lisp_eval(closure, lisp_car(cdr));
  atom_t res = lisp_cons(fst, snd);
  LISP_FREE(snd, cdr, fst, cell);
  return res;
}

/*
 * DEF/SETQ.
 */

static atom_t
lisp_function_def(const atom_t closure, const atom_t cell)
{
  atom_t sym = lisp_car(cell);
  atom_t val = lisp_cdr(cell);
  GLOBALS = lisp_setq(GLOBALS, sym, UP(val));
  LISP_FREE(cell);
  return val;
}

static atom_t
lisp_function_setq(const atom_t closure, const atom_t cell)
{
  atom_t sym = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  atom_t val = lisp_eval(closure, lisp_car(cdr));
  GLOBALS = lisp_setq(GLOBALS, sym, UP(val));
  LISP_FREE(cdr, cell);
  return val;
}

/*
 * Tester functions.
 */

#define PREDICATE_GEN(_n, _o)                                   \
static atom_t                                                   \
lisp_function_is ## _n(const atom_t closure, const atom_t cell) \
{                                                               \
  atom_t car = lisp_eval(closure, lisp_car(cell));              \
  atom_t res = _o(car) ? TRUE : NIL;                            \
  LISP_FREE(car, cell);                                         \
  return UP(res);                                               \
}

PREDICATE_GEN(atm, !IS_PAIR);
PREDICATE_GEN(num, IS_NUMB);
PREDICATE_GEN(str, IS_STRN);
PREDICATE_GEN(sym, IS_SYMB);
PREDICATE_GEN(lst, IS_PAIR);
PREDICATE_GEN(nil, IS_NULL);

/*
 * Logic functions.
 */

#define BINARY_BOOLEAN_GEN(_n, _o)                            \
static atom_t                                                 \
lisp_function_ ## _n(const atom_t closure, const atom_t cell) \
{                                                             \
  atom_t vl0 = lisp_eval(closure, lisp_car(cell));            \
  atom_t cdr = lisp_cdr(cell);                                \
  atom_t vl1 = lisp_eval(closure, lisp_car(cdr));             \
  atom_t res = IS_TRUE(vl0) _o IS_TRUE(vl1) ? TRUE : NIL;     \
  LISP_FREE(vl1, cdr, vl0, cell);                             \
  return UP(res);                                             \
}

BINARY_BOOLEAN_GEN(and, &&);
BINARY_BOOLEAN_GEN(or, ||);

static atom_t
lisp_function_not(const atom_t closure, const atom_t cell)
{
  atom_t car = lisp_eval(closure, lisp_car(cell));
  atom_t res = IS_NULL(car) ? TRUE : NIL;
  LISP_FREE(car, cell);
  return UP(res);
}

/*
 * Arithmetic functions.
 */

#define BINARY_NUMBER_GEN(_n, _o)                             \
static atom_t                                                 \
lisp_function_ ## _n(const atom_t closure, const atom_t cell) \
{                                                             \
  atom_t vl0 = lisp_eval(closure, lisp_car(cell));            \
  atom_t cdr = lisp_cdr(cell);                                \
  atom_t vl1 = lisp_eval(closure, lisp_car(cdr));             \
  atom_t res = lisp_make_number(vl0->number _o vl1->number);  \
  LISP_FREE(vl1, cdr, vl0, cell);                             \
  return res;                                                 \
}

BINARY_NUMBER_GEN(add, +);
BINARY_NUMBER_GEN(sub, -);
BINARY_NUMBER_GEN(mul, *);
BINARY_NUMBER_GEN(div, /);

/*
 * Structural equality.
 */

static atom_t
lisp_function_equ(const atom_t closure, const atom_t cell)
{
  atom_t vl0 = lisp_eval(closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  atom_t vl1 = lisp_eval(closure, lisp_car(cdr));
  atom_t res = lisp_equl(vl0, vl1) ? TRUE : NIL;
  LISP_FREE(vl1, cdr, vl0, cell);
  return UP(res);
}

/*
 * Comparators.
 */

#define BINARY_COMPARE_GEN(_n, _o)                              \
static atom_t                                                   \
lisp_function_ ## _n(const atom_t closure, const atom_t cell)   \
{                                                               \
  atom_t vl0 = lisp_eval(closure, lisp_car(cell));              \
  atom_t cdr = lisp_cdr(cell);                                  \
  atom_t vl1 = lisp_eval(closure, lisp_car(cdr));               \
  atom_t res = vl0->number _o vl1->number ? TRUE : NIL;         \
  LISP_FREE(vl1, cdr, vl0, cell);                               \
  return UP(res);                                               \
}

BINARY_COMPARE_GEN(gt, >);
BINARY_COMPARE_GEN(lt, <);
BINARY_COMPARE_GEN(ge, >=);
BINARY_COMPARE_GEN(le, <=);

/*
 * Conditionals.
 */

typedef atom_t (* ite_case_t)(const atom_t, const atom_t, const atom_t);

static atom_t
lisp_function_ite_then(const atom_t closure, const atom_t thn, const atom_t els)
{
  LISP_FREE(els);
  return lisp_eval(closure, thn);
}

static atom_t
lisp_function_ite_else(const atom_t closure, const atom_t thn, const atom_t els)
{
  LISP_FREE(thn);
  return lisp_eval(closure, els);
}

static ite_case_t lisp_function_ite_table[ATOM_TYPES] =
{
  [T_NIL     ] = lisp_function_ite_else,
  [T_PAIR    ] = lisp_function_ite_else,
  [T_NUMBER  ] = lisp_function_ite_else,
  [T_STRING  ] = lisp_function_ite_else,
  [T_SYMBOL  ] = lisp_function_ite_else,
  [T_TRUE    ] = lisp_function_ite_then,
  [T_WILDCARD] = lisp_function_ite_else,
};

static atom_t
lisp_function_ith(const atom_t closure, const atom_t cell)
{
  atom_t cnd = lisp_eval(closure, lisp_car(cell));
  atom_t cd0 = lisp_cdr(cell);
  atom_t thn = lisp_car(cd0);
  /*
   * Execute the right branch depending on the result.
   */
  atom_type_t type = cnd->type;
  LISP_FREE(cd0, cnd, cell);
  return lisp_function_ite_table[type](closure, thn, UP(NIL));
}

static atom_t
lisp_function_int(const atom_t closure, const atom_t cell)
{
  atom_t res = lisp_eval(closure, lisp_car(cell));
  atom_t cnd = IS_TRUE(res) ? NIL : TRUE;
  atom_t cd0 = lisp_cdr(cell);
  atom_t thn = lisp_car(cd0);
  /*
   * Execute the right branch depending on the result.
   */
  atom_type_t type = cnd->type;
  LISP_FREE(cd0, res, cell);
  return lisp_function_ite_table[type](closure, thn, UP(NIL));
}

static atom_t
lisp_function_ite(const atom_t closure, const atom_t cell)
{
  atom_t cnd = lisp_eval(closure, lisp_car(cell));
  atom_t cd0 = lisp_cdr(cell);
  atom_t thn = lisp_car(cd0);
  atom_t cd1 = lisp_cdr(cd0);
  atom_t els = lisp_car(cd1);
  /*
   * Execute the right branch depending on the result.
   */
  atom_type_t type = cnd->type;
  LISP_FREE(cd1, cd0, cnd, cell);
  return lisp_function_ite_table[type](closure, thn, els);
}

/*
 * Set-up function.
 */

typedef struct _def_t
{
  const char *  name;
  function_t    fun;
  bool          enabled;
}
def_t;

static def_t functions[] = {
  { "*"    , lisp_function_mul  , true },
  { "+"    , lisp_function_add  , true },
  { "-"    , lisp_function_sub  , true },
  { "/"    , lisp_function_div  , true },
  { "="    , lisp_function_equ  , true },
  { ">"    , lisp_function_gt   , true },
  { "<"    , lisp_function_lt   , true },
  { ">="   , lisp_function_ge   , true },
  { "<="   , lisp_function_le   , true },
  { "?"    , lisp_function_ith  , true },
  { "?!"   , lisp_function_int  , true },
  { "?:"   , lisp_function_ite  , true },
  { "and"  , lisp_function_and  , true },
  { "atm?" , lisp_function_isatm, true },
  { "car"  , lisp_function_car  , true },
  { "cdr"  , lisp_function_cdr  , true },
  { "conc" , lisp_function_conc , true },
  { "cons" , lisp_function_cons , true },
  { "def"  , lisp_function_def  , true },
  { "eval" , lisp_function_eval , true },
  { "lst?" , lisp_function_islst, true },
  { "nil?" , lisp_function_isnil, true },
  { "not"  , lisp_function_not  , true },
  { "num?" , lisp_function_isnum, true },
  { "or"   , lisp_function_or   , true },
  { "prog" , lisp_function_prog , true },
  { "quote", lisp_function_quote, true },
  { "setq" , lisp_function_setq , true },
  { "str?" , lisp_function_isstr, true },
  { "sym?" , lisp_function_issym, true },
};

#define FUNCTION_COUNT (sizeof(functions) / sizeof(def_t))

void
lisp_function_register_all()
{
  for (size_t i = 0; i < FUNCTION_COUNT; i += 1) {
    if (functions[i].enabled) {
      atom_t sym = lisp_make_symbol(strdup(functions[i].name));
      atom_t val = lisp_make_number((uintptr_t)functions[i].fun);
      GLOBALS = lisp_setq(GLOBALS, sym, val);
    }
  }
}
