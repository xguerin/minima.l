#include "functions.h"
#include "slab.h"
#include "symbols.h"

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

cell_t
lisp_function_eval(const cell_t cell)
{
  cell_t car = lisp_car(cell);
  lisp_free(1, cell);
  return lisp_eval(car);
}

/*
 * CAR/CDR.
 */

cell_t
lisp_function_car(const cell_t cell)
{
  cell_t result = lisp_car(cell);
  lisp_free(1, cell);
  return result;
}

cell_t
lisp_function_cdr(const cell_t cell)
{
  cell_t result = lisp_cdr(cell);
  lisp_free(1, cell);
  return result;
}

/*
 * CONS/CONC.
 */

cell_t
lisp_function_conc(const cell_t cell)
{
  cell_t fst = lisp_car(cell);
  cell_t cdr = lisp_cdr(cell);
  cell_t snd = lisp_car(cdr);
  cell_t res = lisp_conc(fst, snd);
  lisp_free(2, cell, cdr);
  return res;
}

cell_t
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
 * Tester functions.
 * (fun? 'any)
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

#define MAKE_SYMBOL(__f) (lisp_make_number((uintptr_t)__f))

void
lisp_function_register_all()
{
  lisp_symbol_register("quote", MAKE_SYMBOL(lisp_function_quote));
  lisp_symbol_register("eval" , MAKE_SYMBOL(lisp_function_eval ));

  lisp_symbol_register("car"  , MAKE_SYMBOL(lisp_function_car  ));
  lisp_symbol_register("cdr"  , MAKE_SYMBOL(lisp_function_cdr  ));

  lisp_symbol_register("conc" , MAKE_SYMBOL(lisp_function_conc ));
  lisp_symbol_register("cons" , MAKE_SYMBOL(lisp_function_cons ));

  lisp_symbol_register("inc"  , MAKE_SYMBOL(lisp_function_inc  ));
  lisp_symbol_register("dec"  , MAKE_SYMBOL(lisp_function_dec  ));

  lisp_symbol_register("num?" , MAKE_SYMBOL(lisp_function_isnum));
  lisp_symbol_register("str?" , MAKE_SYMBOL(lisp_function_isstr));
  lisp_symbol_register("sym?" , MAKE_SYMBOL(lisp_function_issym));
  lisp_symbol_register("lst?" , MAKE_SYMBOL(lisp_function_islst));
}
