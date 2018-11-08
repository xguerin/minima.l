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
  X(cell);
  return lisp_eval(closure, car);
}

static atom_t
lisp_function_prog(const atom_t closure, const atom_t cell)
{
  return lisp_prog(closure, cell, UP(NIL));
}

static atom_t
lisp_function_pipe(const atom_t closure, const atom_t cell)
{
  return lisp_pipe(closure, cell, UP(NIL));
}

static atom_t
lisp_function_list(const atom_t closure, const atom_t cell)
{
  return lisp_list(closure, cell);
}

/*
 * CAR/CDR.
 */

static atom_t
lisp_function_car(const atom_t closure, const atom_t cell)
{
  atom_t arg = lisp_eval(closure, lisp_car(cell));
  X(cell);
  atom_t res = lisp_car(arg);
  X(arg);
  return res;
}

static atom_t
lisp_function_cdr(const atom_t closure, const atom_t cell)
{
  atom_t arg = lisp_eval(closure, lisp_car(cell));
  X(cell);
  atom_t res = lisp_cdr(arg);
  X(arg);
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
  X(cell);
  atom_t snd = lisp_eval(closure, lisp_car(cdr));
  X(cdr);
  atom_t res = lisp_conc(fst, snd);
  X(snd);
  return res;
}

static atom_t
lisp_function_cons(const atom_t closure, const atom_t cell)
{
  atom_t fst = lisp_eval(closure, lisp_car(cell));
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  atom_t snd = lisp_eval(closure, lisp_car(cdr));
  X(cdr);
  atom_t res = lisp_cons(fst, snd);
  X(fst); X(snd);
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
  X(cell);
  return val;
}

static atom_t
lisp_function_setq(const atom_t closure, const atom_t cell)
{
  atom_t sym = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  atom_t val = lisp_eval(closure, lisp_car(cdr));
  X(cdr);
  GLOBALS = lisp_setq(GLOBALS, sym, UP(val));
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
  X(cell);                                                      \
  atom_t res = _o(car) ? TRUE : NIL;                            \
  X(car);                                                       \
  return UP(res);                                               \
}

PREDICATE_GEN(atm, !IS_PAIR);
PREDICATE_GEN(num, IS_NUMB);
PREDICATE_GEN(str, IS_STRN);
PREDICATE_GEN(sym, IS_SETQ);
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
  X(cell);                                                    \
  atom_t vl1 = lisp_eval(closure, lisp_car(cdr));             \
  X(cdr);                                                     \
  atom_t res = IS_TRUE(vl0) _o IS_TRUE(vl1) ? TRUE : NIL;     \
  X(vl0); X(vl1);                                             \
  return UP(res);                                             \
}

BINARY_BOOLEAN_GEN(and, &&);
BINARY_BOOLEAN_GEN(or, ||);

static atom_t
lisp_function_not(const atom_t closure, const atom_t cell)
{
  atom_t car = lisp_eval(closure, lisp_car(cell));
  X(cell);
  atom_t res = IS_NULL(car) ? TRUE : NIL;
  X(car);
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
  X(cell);                                                    \
  atom_t vl1 = lisp_eval(closure, lisp_car(cdr));             \
  X(cdr);                                                     \
  atom_t res = lisp_make_number(vl0->number _o vl1->number);  \
  X(vl0); X(vl1);                                             \
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
  X(cell);
  atom_t vl1 = lisp_eval(closure, lisp_car(cdr));
  X(cdr);
  atom_t res = lisp_equl(vl0, vl1) ? TRUE : NIL;
  X(vl0); X(vl1);
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
  X(cell);                                                      \
  atom_t vl1 = lisp_eval(closure, lisp_car(cdr));               \
  X(cdr);                                                       \
  atom_t res = vl0->number _o vl1->number ? TRUE : NIL;         \
  X(vl0); X(vl1);                                               \
  return UP(res);                                               \
}

BINARY_COMPARE_GEN(gt, >);
BINARY_COMPARE_GEN(lt, <);
BINARY_COMPARE_GEN(ge, >=);
BINARY_COMPARE_GEN(le, <=);

/*
 * Conditionals.
 */

static atom_t
lisp_function_ith(const atom_t closure, const atom_t cell)
{
  /*
   * Evaluate the condition expression.
   */
  atom_t cnd = lisp_eval(closure, lisp_car(cell));
  atom_type_t type = cnd->type;
  X(cnd);
  /*
   * Get the THEN branch.
   */
  atom_t cd0 = lisp_cdr(cell);
  X(cell);
  atom_t thn = lisp_car(cd0);
  X(cd0);
  /*
   * Execute the THEN branch.
   */
  if (type == T_TRUE) {
    return lisp_eval(closure, thn);
  }
  /*
   * Clean-up.
   */
  X(thn);
  return UP(NIL);
}

static atom_t
lisp_function_int(const atom_t closure, const atom_t cell)
{
  /*
   * Evaluate the condition expression.
   */
  atom_t res = lisp_eval(closure, lisp_car(cell));
  atom_t cnd = IS_TRUE(res) ? NIL : TRUE;
  atom_type_t type = cnd->type;
  X(res);
  /*
   * Get the THEN branch.
   */
  atom_t cd0 = lisp_cdr(cell);
  X(cell);
  atom_t thn = lisp_car(cd0);
  X(cd0);
  /*
   * Execute the THEN branch.
   */
  if (type == T_NIL) {
    return lisp_eval(closure, thn);
  }
  /*
   * Clean-up.
   */
  X(thn);
  return UP(NIL);
}

static atom_t
lisp_function_ite(const atom_t closure, const atom_t cell)
{
  /*
   * Evaluate the condition expression.
   */
  atom_t cnd = lisp_eval(closure, lisp_car(cell));
  atom_type_t type = cnd->type;
  X(cnd);
  atom_t cd0 = lisp_cdr(cell);
  X(cell);
  /*
   * Evaluate the THEN branch if TRUE.
   */
  if (type == T_TRUE) {
    atom_t thn = lisp_car(cd0);
    X(cd0);
    return lisp_eval(closure, thn);
  }
  /*
   * Or evaluate the ELSE branch.
   */
  else {
    atom_t cd1 = lisp_cdr(cd0);
    X(cd0);
    atom_t els = lisp_car(cd1);
    X(cd1);
    return lisp_eval(closure, els);
  }
}

/*
 * Debug functions.
 */

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
  { "::"   , lisp_function_list , true },
  { "<"    , lisp_function_lt   , true },
  { "<="   , lisp_function_le   , true },
  { "="    , lisp_function_equ  , true },
  { ">"    , lisp_function_gt   , true },
  { ">="   , lisp_function_ge   , true },
  { "?!"   , lisp_function_int  , true },
  { "?"    , lisp_function_ith  , true },
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
  { "|"    , lisp_function_pipe , true },
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
