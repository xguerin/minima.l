#include <lisp/lisp.h>

static cell_t
lisp_function_quote(const cell_t cell)
{
  return lisp_dup(cell);
}

static cell_t
lisp_function_inc(const cell_t cell)
{
  /*
   * Grab the value.
   */
  if (GET_TYPE(cell->car) != T_LIST) {
    return lisp_make_nil();
  }
  cell_t val = lisp_dup(GET_PNTR(cell_t, cell->car));
  /*
   * If it's a list, evaluate it.
   */
  if (GET_TYPE(val->car) == T_LIST) {
    cell_t result;
    bool res = lisp_eval(val, &result);
    lisp_free(1, val);
    if (!res) return lisp_make_nil();
    val = result;
  }
  /*
   * If the value is not a number, error.
   */
  if (GET_TYPE(val->car) != T_NUMBER) {
    lisp_free(1, val);
    return lisp_make_nil();
  }
  /*
   * Decrease the number and create the result.
   */
  cell_t n = lisp_make_number(GET_NUMB(val->car) + 1);
  lisp_free(1, val);
  return n;
}

static cell_t
lisp_function_dec(const cell_t cell)
{
  /*
   * Grab the value.
   */
  if (GET_TYPE(cell->car) != T_LIST) {
    return lisp_make_nil();
  }
  cell_t val = lisp_dup(GET_PNTR(cell_t, cell->car));
  /*
   * If it's a list, evaluate it.
   */
  if (GET_TYPE(val->car) == T_LIST) {
    cell_t result;
    bool res = lisp_eval(val, &result);
    lisp_free(1, val);
    if (!res) return lisp_make_nil();
    val = result;
  }
  /*
   * If the value is not a number, error.
   */
  if (GET_TYPE(val->car) != T_NUMBER) {
    lisp_free(1, val);
    return lisp_make_nil();
  }
  /*
   * Decrease the number and create the result.
   */
  cell_t n = lisp_make_number(GET_NUMB(val->car) - 1);
  lisp_free(1, val);
  return n;
}

void
lisp_function_register_all()
{
  lisp_function_register("quote", lisp_function_quote);
  lisp_function_register("inc"  , lisp_function_inc);
  lisp_function_register("dec"  , lisp_function_dec);
}
