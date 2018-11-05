#include "functions.h"
#include "slab.h"
#include <string.h>

/*
 * QUOTE.
 */

cell_t
lisp_function_quote(const cell_t closure, const cell_t cell)
{
  return cell;
}

/*
 * EVAL.
 */

static cell_t
lisp_function_eval(const cell_t closure, const cell_t cell)
{
  cell_t car = lisp_car(cell);
  cell_t res = lisp_eval(closure, car);
  LISP_FREE(cell);
  return res;
}

static cell_t
lisp_function_prog(const cell_t closure, const cell_t cell)
{
  return lisp_prog(closure, cell, lisp_make_nil());
}

/*
 * CAR/CDR.
 */

static cell_t
lisp_function_car(const cell_t closure, const cell_t cell)
{
  cell_t arg = lisp_eval(closure, lisp_car(cell));
  cell_t res = lisp_car(arg);
  LISP_FREE(cell, arg);
  return res;
}

static cell_t
lisp_function_cdr(const cell_t closure, const cell_t cell)
{
  cell_t arg = lisp_eval(closure, lisp_car(cell));
  cell_t res = lisp_cdr(arg);
  LISP_FREE(cell, arg);
  return res;
}

/*
 * CONS/CONC.
 */

static cell_t
lisp_function_conc(const cell_t closure, const cell_t cell)
{
  cell_t fst = lisp_eval(closure, lisp_car(cell));
  cell_t cdr = lisp_cdr(cell);
  cell_t snd = lisp_eval(closure, lisp_car(cdr));
  cell_t res = lisp_conc(fst, snd);
  LISP_FREE(cell, cdr);
  return res;
}

static cell_t
lisp_function_cons(const cell_t closure, const cell_t cell)
{
  cell_t fst = lisp_eval(closure, lisp_car(cell));
  cell_t cdr = lisp_cdr(cell);
  cell_t snd = lisp_eval(closure, lisp_car(cdr));
  cell_t res = lisp_cons(fst, snd);
  LISP_FREE(cell, fst, cdr, snd);
  return res;
}

/*
 * DEF/SETQ.
 */

static cell_t
lisp_function_def(const cell_t closure, const cell_t cell)
{
  cell_t sym = lisp_car(cell);
  cell_t val = lisp_cdr(cell);
  GLOBALS = lisp_setq(GLOBALS, sym, val);
  LISP_FREE(sym, cell);
  return val;
}

static cell_t
lisp_function_setq(const cell_t closure, const cell_t cell)
{
  cell_t sym = lisp_car(cell);
  cell_t cdr = lisp_cdr(cell);
  cell_t val = lisp_eval(closure, lisp_car(cdr));
  GLOBALS = lisp_setq(GLOBALS, sym, val);
  LISP_FREE(cdr, sym, cell);
  return val;
}

/*
 * Tester functions.
 */

static cell_t
lisp_function_isatm(const cell_t closure, const cell_t cell)
{
  cell_t car = lisp_eval(closure, lisp_car(cell));
  cell_t res = IS_LIST(car) ? lisp_make_nil() : lisp_make_true();
  LISP_FREE(car, cell);
  return res;
}

static cell_t
lisp_function_isnum(const cell_t closure, const cell_t cell)
{
  cell_t car = lisp_eval(closure, lisp_car(cell));
  cell_t res = IS_NUMB(car) ? lisp_make_true() : lisp_make_nil();
  LISP_FREE(car, cell);
  return res;
}

static cell_t
lisp_function_isstr(const cell_t closure, const cell_t cell)
{
  cell_t car = lisp_eval(closure, lisp_car(cell));
  cell_t res = IS_STRN(car) ? lisp_make_true() : lisp_make_nil();
  LISP_FREE(car, cell);
  return res;
}

static cell_t
lisp_function_issym(const cell_t closure, const cell_t cell)
{
  cell_t car = lisp_eval(closure, lisp_car(cell));
  cell_t res = IS_SYMB(car) ? lisp_make_true() : lisp_make_nil();
  LISP_FREE(car, cell);
  return res;
}

static cell_t
lisp_function_islst(const cell_t closure, const cell_t cell)
{
  cell_t car = lisp_eval(closure, lisp_car(cell));
  cell_t res = IS_LIST(car) ? lisp_make_true() : lisp_make_nil();
  LISP_FREE(car, cell);
  return res;
}

static cell_t
lisp_function_isnil(const cell_t closure, const cell_t cell)
{
  cell_t car = lisp_eval(closure, lisp_car(cell));
  cell_t res = IS_NULL(car) ? lisp_make_true() : lisp_make_nil();
  LISP_FREE(car, cell);
  return res;
}

/*
 * Logic functions.
 */

static cell_t
lisp_function_and(const cell_t closure, const cell_t cell)
{
  cell_t vl0 = lisp_eval(closure, lisp_car(cell));
  cell_t cdr = lisp_cdr(cell);
  cell_t vl1 = lisp_eval(closure, lisp_car(cdr));
  cell_t res = IS_TRUE(vl0) && IS_TRUE(vl1) ? lisp_make_true() : lisp_make_nil();
  LISP_FREE(cell, vl0, cdr, vl1);
  return res;
}

static cell_t
lisp_function_or(const cell_t closure, const cell_t cell)
{
  cell_t vl0 = lisp_eval(closure, lisp_car(cell));
  cell_t cdr = lisp_cdr(cell);
  cell_t vl1 = lisp_eval(closure, lisp_car(cdr));
  cell_t res = IS_TRUE(vl0) || IS_TRUE(vl1) ? lisp_make_true() : lisp_make_nil();
  LISP_FREE(cell, vl0, cdr, vl1);
  return res;
}

static cell_t
lisp_function_not(const cell_t closure, const cell_t cell)
{
  cell_t car = lisp_eval(closure, lisp_car(cell));
  cell_t res = IS_NULL(car) ? lisp_make_true() : lisp_make_nil();
  LISP_FREE(car, cell);
  return res;
}

/*
 * Arithmetic functions.
 */

static cell_t
lisp_function_add(const cell_t closure, const cell_t cell)
{
  cell_t vl0 = lisp_eval(closure, lisp_car(cell));
  cell_t cdr = lisp_cdr(cell);
  cell_t vl1 = lisp_eval(closure, lisp_car(cdr));
  cell_t res = lisp_make_number(GET_NUMB(vl0->car) + GET_NUMB(vl1->car));
  LISP_FREE(cell, vl0, cdr, vl1);
  return res;
}

static cell_t
lisp_function_sub(const cell_t closure, const cell_t cell)
{
  cell_t vl0 = lisp_eval(closure, lisp_car(cell));
  cell_t cdr = lisp_cdr(cell);
  cell_t vl1 = lisp_eval(closure, lisp_car(cdr));
  cell_t res = lisp_make_number(GET_NUMB(vl0->car) - GET_NUMB(vl1->car));
  LISP_FREE(cell, vl0, cdr, vl1);
  return res;
}

static cell_t
lisp_function_mul(const cell_t closure, const cell_t cell)
{
  cell_t vl0 = lisp_eval(closure, lisp_car(cell));
  cell_t cdr = lisp_cdr(cell);
  cell_t vl1 = lisp_eval(closure, lisp_car(cdr));
  cell_t res = lisp_make_number(GET_NUMB(vl0->car) * GET_NUMB(vl1->car));
  LISP_FREE(cell, vl0, cdr, vl1);
  return res;
}

static cell_t
lisp_function_div(const cell_t closure, const cell_t cell)
{
  cell_t vl0 = lisp_eval(closure, lisp_car(cell));
  cell_t cdr = lisp_cdr(cell);
  cell_t vl1 = lisp_eval(closure, lisp_car(cdr));
  cell_t res = lisp_make_number(GET_NUMB(vl0->car) / GET_NUMB(vl1->car));
  LISP_FREE(cell, vl0, cdr, vl1);
  return res;
}

static cell_t
lisp_function_equ(const cell_t closure, const cell_t cell)
{
  cell_t vl0 = lisp_eval(closure, lisp_car(cell));
  cell_t cdr = lisp_cdr(cell);
  cell_t vl1 = lisp_eval(closure, lisp_car(cdr));
  bool res = lisp_equl(vl0, vl1);
  LISP_FREE(vl1, cdr, vl0, cell);
  return res ? lisp_make_true() : lisp_make_nil();
}

/*
 * Conditionals.
 */

static cell_t
lisp_function_ite_then(const cell_t closure, const cell_t thn, const cell_t els)
{
  LISP_FREE(els);
  return lisp_eval(closure, thn);
}

static cell_t
lisp_function_ite_else(const cell_t closure, const cell_t thn, const cell_t els)
{
  LISP_FREE(thn);
  return lisp_eval(closure, els);
}

static cell_t (* lisp_function_ite_table[8])(const cell_t, const cell_t, const cell_t) =
{
  [T_NIL          ] = lisp_function_ite_else,
  [T_LIST         ] = lisp_function_ite_else,
  [T_NUMBER       ] = lisp_function_ite_else,
  [T_STRING       ] = lisp_function_ite_else,
  [T_SYMBOL       ] = lisp_function_ite_else,
  [T_SYMBOL_INLINE] = lisp_function_ite_else,
  [T_TRUE         ] = lisp_function_ite_then,
  [T_WILDCARD     ] = lisp_function_ite_else,
};

static cell_t
lisp_function_ith(const cell_t closure, const cell_t cell)
{
  cell_t cnd = lisp_eval(closure, lisp_car(cell));
  cell_t cd0 = lisp_cdr(cell);
  cell_t thn = lisp_car(cd0);
  /*
   * Execute the right branch depending on the result.
   */
  cell_type_t type = GET_TYPE(cnd->car);
  LISP_FREE(cd0, cnd, cell);
  return lisp_function_ite_table[type](closure, thn, lisp_make_nil());
}

static cell_t
lisp_function_int(const cell_t closure, const cell_t cell)
{
  cell_t res = lisp_eval(closure, lisp_car(cell));
  TRACE_SEXP(res);
  cell_t cnd = IS_TRUE(res) ? lisp_make_nil() : lisp_make_true();
  TRACE_SEXP(cnd);
  cell_t cd0 = lisp_cdr(cell);
  cell_t thn = lisp_car(cd0);
  /*
   * Execute the right branch depending on the result.
   */
  cell_type_t type = GET_TYPE(cnd->car);
  LISP_FREE(cd0, res, cnd, cell);
  return lisp_function_ite_table[type](closure, thn, lisp_make_nil());
}

static cell_t
lisp_function_ite(const cell_t closure, const cell_t cell)
{
  cell_t cnd = lisp_eval(closure, lisp_car(cell));
  cell_t cd0 = lisp_cdr(cell);
  cell_t thn = lisp_car(cd0);
  cell_t cd1 = lisp_cdr(cd0);
  cell_t els = lisp_car(cd1);
  /*
   * Execute the right branch depending on the result.
   */
  cell_type_t type = GET_TYPE(cnd->car);
  LISP_FREE(cd1, cd0, cnd, cell);
  return lisp_function_ite_table[type](closure, thn, els);
}

/*
 * Set-up function.
 */

typedef struct _def_t
{
  const char * name;
  function_t   fun;
}
def_t;

static def_t functions[] = {
  { "*"    , lisp_function_mul   },
  { "+"    , lisp_function_add   },
  { "-"    , lisp_function_sub   },
  { "/"    , lisp_function_div   },
  { "="    , lisp_function_equ   },
  { "?"    , lisp_function_ith   },
  { "?!"   , lisp_function_int   },
  { "?:"   , lisp_function_ite   },
  { "and"  , lisp_function_and   },
  { "atm?" , lisp_function_isatm },
  { "car"  , lisp_function_car   },
  { "cdr"  , lisp_function_cdr   },
  { "conc" , lisp_function_conc  },
  { "cons" , lisp_function_cons  },
  { "def"  , lisp_function_def   },
  { "eval" , lisp_function_eval  },
  { "lst?" , lisp_function_islst },
  { "nil?" , lisp_function_isnil },
  { "not"  , lisp_function_not   },
  { "num?" , lisp_function_isnum },
  { "or"   , lisp_function_or    },
  { "prog" , lisp_function_prog  },
  { "quote", lisp_function_quote },
  { "setq" , lisp_function_setq  },
  { "str?" , lisp_function_isstr },
  { "sym?" , lisp_function_issym },
};

#define FUNCTION_COUNT (sizeof(functions) / sizeof(def_t))

void
lisp_function_register_all()
{
  for (size_t i = 0; i < FUNCTION_COUNT; i += 1) {
    cell_t a = lisp_make_symbol(functions[i].name);
    cell_t b = lisp_make_number((uintptr_t)functions[i].fun);
    GLOBALS = lisp_setq(GLOBALS, a, b);
    LISP_FREE(a, b);
  }
}
