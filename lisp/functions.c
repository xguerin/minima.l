#include "functions.h"
#include "slab.h"
#include <string.h>

/*
 * QUOTE.
 */

cell_t
lisp_function_quote(const cell_t cell)
{
  return cell;
}

/*
 * EVAL.
 */

static cell_t
lisp_function_eval(const cell_t cell)
{
  cell_t car = lisp_car(cell);
  lisp_free(1, cell);
  return lisp_eval(car);
}

/*
 * CAR/CDR.
 */

static cell_t
lisp_function_car(const cell_t cell)
{
  cell_t result = lisp_car(cell);
  lisp_free(1, cell);
  return result;
}

static cell_t
lisp_function_cdr(const cell_t cell)
{
  cell_t result = lisp_cdr(cell);
  lisp_free(1, cell);
  return result;
}

/*
 * CONS/CONC.
 */

static cell_t
lisp_function_conc(const cell_t cell)
{
  cell_t fst = lisp_car(cell);
  cell_t cdr = lisp_cdr(cell);
  cell_t snd = lisp_car(cdr);
  cell_t res = lisp_conc(fst, snd);
  lisp_free(2, cell, cdr);
  return res;
}

static cell_t
lisp_function_cons(const cell_t cell)
{
  cell_t fst = lisp_car(cell);
  cell_t cdr = lisp_cdr(cell);
  cell_t snd = lisp_car(cdr);
  cell_t res = lisp_cons(fst, snd);
  lisp_free(4, cell, fst, cdr, snd);
  return res;
}

/*
 * SETQ.
 */

static cell_t
lisp_function_setq(const cell_t cell)
{
  cell_t sym = lisp_car(cell);
  /*
   * Check if the first argument is a symbol.
   */
  if (GET_TYPE(sym->car) != T_SYMBOL && GET_TYPE(sym->car) != T_SYMBOL_INLINE) {
    lisp_free(2, sym, cell);
    return lisp_make_nil();
  }
  /*
   * Call SETQ.
   */
  cell_t cdr = lisp_cdr(cell);
  cell_t val = lisp_car(cdr);
  cell_t res = lisp_setq(sym, val);
  lisp_free(3, cell, sym, cdr);
  return res;
}

/*
 * Tester functions.
 */

static cell_t
lisp_function_isnum(const cell_t cell)
{
  cell_t car = lisp_car(cell);
  cell_t res = IS_NUMB(car) ? lisp_make_true() : lisp_make_nil();
  lisp_free(2, cell, car);
  return res;
}

static cell_t
lisp_function_isstr(const cell_t cell)
{
  cell_t car = lisp_car(cell);
  cell_t res = IS_STRN(car) ? lisp_make_true() : lisp_make_nil();
  lisp_free(2, cell, car);
  return res;
}

static cell_t
lisp_function_issym(const cell_t cell)
{
  cell_t car = lisp_car(cell);
  cell_t res = IS_SYMB(car) ? lisp_make_true() : lisp_make_nil();
  lisp_free(2, cell, car);
  return res;
}

static cell_t
lisp_function_islst(const cell_t cell)
{
  cell_t car = lisp_car(cell);
  cell_t res = IS_LIST(car) ? lisp_make_true() : lisp_make_nil();
  lisp_free(2, cell, car);
  return res;
}

/*
 * Other functions.
 */

static cell_t
lisp_function_inc(const cell_t cell)
{
  cell_t val = lisp_car(cell);
  cell_t res = GET_TYPE(val->car) != T_NUMBER ?
    lisp_make_nil() :
    lisp_make_number(GET_NUMB(val->car) + 1);
  lisp_free(2, val, cell);
  return res;
}

static cell_t
lisp_function_dec(const cell_t cell)
{
  cell_t val = lisp_car(cell);
  cell_t res = GET_TYPE(val->car) != T_NUMBER ?
    lisp_make_nil() :
    lisp_make_number(GET_NUMB(val->car) - 1);
  lisp_free(2, val, cell);
  return res;
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
  { "setq" , lisp_function_setq  },
  { "inc"  , lisp_function_inc   },
  { "dec"  , lisp_function_dec   },
  { "num?" , lisp_function_isnum },
  { "str?" , lisp_function_isstr },
  { "sym?" , lisp_function_issym },
  { "lst?" , lisp_function_islst },
};

#define FUNCTION_COUNT (sizeof(functions) / sizeof(def_t))

void
lisp_function_register_all()
{
  /*
   * Create the global symbol list.
   */
  globals = lisp_make_nil();
  /*
   * Create all the symbols.
   */
  for (size_t i = 0; i < FUNCTION_COUNT; i += 1) {
    cell_t a = lisp_make_symbol(functions[i].name);
    cell_t b = lisp_make_number((uintptr_t)functions[i].fun);
    cell_t c = lisp_cons(a, b);
    cell_t d = lisp_make_list(c);
    globals = lisp_conc(globals, d);
  }
}
