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
 * DEFN/SETQ.
 */

static cell_t
lisp_function_defn(const cell_t closure, const cell_t cell)
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
lisp_function_isnum(const cell_t closure, const cell_t cell)
{
  cell_t car = lisp_eval(closure, lisp_car(cell));
  cell_t res = IS_NUMB(car) ? lisp_make_true() : NIL;
  LISP_FREE(car, cell);
  return res;
}

static cell_t
lisp_function_isstr(const cell_t closure, const cell_t cell)
{
  cell_t car = lisp_eval(closure, lisp_car(cell));
  cell_t res = IS_STRN(car) ? lisp_make_true() : NIL;
  LISP_FREE(car, cell);
  return res;
}

static cell_t
lisp_function_issym(const cell_t closure, const cell_t cell)
{
  cell_t car = lisp_eval(closure, lisp_car(cell));
  cell_t res = IS_SYMB(car) ? lisp_make_true() : NIL;
  LISP_FREE(car, cell);
  return res;
}

static cell_t
lisp_function_islst(const cell_t closure, const cell_t cell)
{
  cell_t car = lisp_eval(closure, lisp_car(cell));
  cell_t res = IS_LIST(car) ? lisp_make_true() : NIL;
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
  return res ? lisp_make_true() : NIL;
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
  { "quote", lisp_function_quote },
  { "eval" , lisp_function_eval  },
  { "car"  , lisp_function_car   },
  { "cdr"  , lisp_function_cdr   },
  { "conc" , lisp_function_conc  },
  { "cons" , lisp_function_cons  },
  { "defn" , lisp_function_defn  },
  { "setq" , lisp_function_setq  },
  { "+"    , lisp_function_add   },
  { "-"    , lisp_function_sub   },
  { "*"    , lisp_function_mul   },
  { "/"    , lisp_function_div   },
  { "="    , lisp_function_equ   },
  { "num?" , lisp_function_isnum },
  { "str?" , lisp_function_isstr },
  { "sym?" , lisp_function_issym },
  { "lst?" , lisp_function_islst },
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
