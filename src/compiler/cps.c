#include <mnml/cps.h>
#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

/*
 * Utility functions.
 */

static atom_t
cps_make_placeholder(const size_t counter) {
  char buffer[LISP_SYMBOL_LENGTH];
  int len = snprintf(buffer, LISP_SYMBOL_LENGTH, "_%ld", counter);
  MAKE_SYMBOL_STATIC(sym, buffer, len);
  return lisp_make_symbol(sym);
}

/*
 * Recursively swap funcall with placeholders.
 */

static atom_t
cps_swap(const atom_t cell, size_t* counter, atom_t* spill);

static atom_t
cps_swap_any(const atom_t cell, size_t* counter, atom_t* spill)
{
  TRACE_SEXP(cell);
  /*
   * If it's NIL, return.
   */
  if (IS_NULL(cell)) {
    return cell;
  }
  /*
   * Grab CAR and CDR.
   */
  atom_t car = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  /*
   * Call swap recursively.
   */
  atom_t curswap = cps_swap(cdr, counter, spill);
  /*
   * If CAR is not a list, just CONS and return.
   */
  if (!IS_PAIR(car)) {
    atom_t result = lisp_cons(car, curswap);
    X(car); X(curswap);
    return result;
  }
  /*
   * Generate a symbol for the placeholder and increment the counter.
   */
  atom_t sym_n = cps_make_placeholder(*counter);
  *counter += 1;
  /*
   * Call swap on the argument.
   */
  atom_t recspill = UP(NIL);
  atom_t recswap = cps_swap(car, counter, &recspill);
  /*
   * CONS the result of the swap into the spill.
   */
  atom_t spill0 = *spill;
  atom_t spill1 = lisp_cons(recswap, spill0);
  X(recswap); X(spill0);
  *spill = lisp_conc(recspill, spill1);
  X(recspill); X(spill1);
  /*
   * Append the symbol and return.
   */
  atom_t result = lisp_cons(sym_n, curswap);
  X(sym_n); X(curswap);
  return result;
}

static atom_t
cps_swap_ite(const atom_t cell, size_t* counter, atom_t* spill)
{
  TRACE_SEXP(cell);
  /*
   * Split the ITE expression.
   */
  atom_t symb = lisp_car(cell);
  atom_t cdr0 = lisp_cdr(cell);
  atom_t cond = lisp_car(cdr0);
  atom_t cdr1 = lisp_cdr(cdr0);
  atom_t ifth = lisp_car(cdr1);
  atom_t cdr2 = lisp_cdr(cdr1);
  atom_t ifel = lisp_car(cdr2);
  X(cell); X(cdr0); X(cdr1); X(cdr2);
  /*
   * Call lift on else.
   */
  size_t pad0 = *counter;
  atom_t spel = UP(NIL);
  atom_t nxel = lisp_append(spel, cps_swap(ifel, &pad0, &spel));
  TRACE_SEXP(nxel);
  /*
   * Call lift on then.
   */
  size_t pad1 = *counter;
  atom_t spth = UP(NIL);
  atom_t nxth = lisp_append(spth, cps_swap(ifth, &pad1, &spth));
  TRACE_SEXP(nxth);
  /*
   * Update the counter depending on the results.
   */
  *counter = pad0 > pad1 ? pad0 : pad1;
  /*
   * Reconstruct IF+COND.
   */
  atom_t clst = lisp_cons(cond, NIL);
  atom_t nxif = lisp_cons(symb, clst);
  X(cond); X(NIL); X(clst); X(symb);
  /*
   * Call swap on it and append the lifted then and else.
   */
  atom_t nxfn = cps_swap_any(nxif, counter, spill);
  atom_t rslt = lisp_append(lisp_append(nxfn, nxth), nxel);
  return rslt;
}

static atom_t
cps_swap(const atom_t cell, size_t* counter, atom_t* spill)
{
  TRACE_SEXP(cell);
  /*
   * If it's NIL or not a PAIR, return.
   */
  if (IS_NULL(cell) || ! IS_PAIR(cell)) {
    return cell;
  }
  /*
   * If the funcall symbol is IF, call the appropriate SWAP.
   */
  if (LISP_SYMBOL_MATCH(cell->pair.car, "if")) {
    return cps_swap_ite(cell, counter, spill);
  }
  /*
   * Call the default SWAP.
   */
  return cps_swap_any(cell, counter, spill);
}

/*
 * Recursively decorate funcalls with lambdas.
 */

static atom_t
cps_wrap(const atom_t cell, const size_t pad, size_t* counter);

static atom_t
cps_wrap_any(const atom_t cell, const size_t max, size_t* counter)
{
  /*
   * Generate a symbol for the placeholder and increment the counter.
   */
  atom_t sym_n = cps_make_placeholder(*counter);
  *counter += 1;
  /*
   * Create the arglist.
   */
  atom_t tmp = lisp_cons(sym_n, NIL);
  atom_t arg = lisp_cons(tmp, NIL);
  X(sym_n); X(tmp);
  /*
   * Append the arglist to the funcall.
   */
  atom_t cn0 = lisp_append(arg, cell);
  /*
   * Create the lambda call.
   */
  MAKE_SYMBOL_STATIC(sym_l, "\\", 1);
  atom_t lbd = lisp_make_symbol(sym_l);
  atom_t fun = lisp_cons(lbd, cn0);
  /*
   * Return the lambda call.
   */
  X(lbd); X(cn0);
  return fun;
}

static atom_t
cps_wrap_ite(const atom_t cell, const size_t max, size_t* counter)
{
  /*
   * Split the ITE expression.
   */
  atom_t symb = lisp_car(cell);
  atom_t cdr0 = lisp_cdr(cell);
  atom_t cond = lisp_car(cdr0);
  atom_t cdr1 = lisp_cdr(cdr0);
  atom_t ifth = lisp_car(cdr1);
  atom_t cdr2 = lisp_cdr(cdr1);
  atom_t ifel = lisp_car(cdr2);
  X(cell); X(cdr0); X(cdr1); X(cdr2);
  /*
   * Wrap the swapped ELSE.
   */
  size_t cnt0 = *counter;
  atom_t nxel = cps_wrap(ifel, max, &cnt0);
  /*
   * Wrap the swapped THEN.
   */
  size_t cnt1 = *counter;
  atom_t nxth = cps_wrap(ifth, max, &cnt1);
  /*
   * Update the counter depending on the results.
   */
  *counter = (cnt0 > cnt1 ? cnt0 : cnt1) - 1;
  /*
   * Generate a symbol for the placeholder and increment the counter.
   */
  atom_t sym_n = cps_make_placeholder(*counter);
  *counter += 1;
  /*
   * Reconstruct IF+COND.
   */
  atom_t clst = lisp_cons(cond, NIL);
  atom_t temp = lisp_cons(symb, clst);
  X(cond); X(NIL); X(clst); X(symb);
  /*
   * Append THEN and ELSE.
   */
  atom_t nxif = lisp_append(lisp_append(temp, nxth), nxel);
  /*
   * Create the arglist.
   */
  atom_t tmp = lisp_cons(sym_n, NIL);
  atom_t arg = lisp_cons(tmp, NIL);
  X(sym_n); X(tmp);
  /*
   * Append the arglist to the funcall.
   */
  atom_t cn0 = lisp_append(arg, nxif);
  /*
   * Create the lambda call.
   */
  MAKE_SYMBOL_STATIC(sym_l, "\\", 1);
  atom_t lbd = lisp_make_symbol(sym_l);
  atom_t fun = lisp_cons(lbd, cn0);
  /*
   * Return the lambda call.
   */
  X(lbd); X(cn0);
  return fun;
}

static atom_t
cps_wrap(const atom_t cell, const size_t max, size_t* counter)
{
  TRACE_SEXP(cell);
  /*
   * If it's NIL, return.
   */
  if (IS_NULL(cell)) {
    atom_t sym_k = cps_make_placeholder(max);
    atom_t result = lisp_cons(sym_k, cell);
    X(sym_k); X(cell);
    return result;
  }
  /*
   * Grab CAR and CDR.
   */
  atom_t car = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  /*
   * Call swap recursively.
   */
  atom_t curwrap = cps_wrap(cdr, max, counter);
  /*
   * Generate the lambda call.
   */
  atom_t fun;
  if (LISP_SYMBOL_MATCH(car->pair.car, "if")) {
    fun = cps_wrap_ite(car, max, counter);
  } else {
    fun = cps_wrap_any(car, max, counter);
  }
  /*
   * Construct the result.
   */
  atom_t res = lisp_cons(fun, curwrap);
  X(fun); X(curwrap);
  return res;
}

/*
 * Convert a funcall body in direct style into a list of lambdas.
 * Each lambda at index N uses the lambda at index N + 1 as a continuation.
 */

atom_t
lisp_cps_lift(const atom_t cell, const size_t pad)
{
  size_t max = pad;
  atom_t spl = UP(NIL);
  atom_t acc = cps_swap(cell, &max, &spl);
  atom_t fns = lisp_append(spl, acc);
  size_t cnt = pad;
  return cps_wrap(fns, max, &cnt);
}

/*
 * Combine lambdas generated by lisp_cps_convert together.
 */

static atom_t
cps_bind_ite(const atom_t body, const atom_t cell)
{
  /*
   * Split the ITE expression.
   */
  atom_t symb = lisp_car(body);
  atom_t cdr0 = lisp_cdr(body);
  atom_t cond = lisp_car(cdr0);
  atom_t cdr1 = lisp_cdr(cdr0);
  atom_t ifth = lisp_car(cdr1);
  atom_t cdr2 = lisp_cdr(cdr1);
  atom_t ifel = lisp_car(cdr2);
  X(body); X(cdr0); X(cdr1); X(cdr2);
  /*
   * Bind THEN and remove the lambda prologue.
   */
  atom_t bndt = lisp_cps_bind(ifth);
  SPLIT_LAMBDA(bndt, tsym, targ, tbdy);
  X(tsym); X(targ);
  /*
   * Bind ELSE and remove the lambda prologue.
   */
  atom_t bnde = lisp_cps_bind(ifel);
  SPLIT_LAMBDA(bnde, esym, earg, ebdy);
  X(esym); X(earg);
  /*
   * Reconstruct IF+COND.
   */
  atom_t clst = lisp_cons(cond, NIL);
  atom_t temp = lisp_cons(symb, clst);
  X(cond); X(NIL); X(clst); X(symb);
  /*
   * Append THEN and ELSE.
   */
  X(cell);
  return lisp_append(lisp_append(temp, tbdy), ebdy);
}

static atom_t
cps_bind_any(const atom_t body, const atom_t cell)
{
  return lisp_append(body, cell);
}

static atom_t
cps_bind(const atom_t body, const atom_t cell)
{
  atom_t result;
  if (LISP_SYMBOL_MATCH(cell->pair.car, "if")) {
    result = cps_bind_ite(body, cell);
  } else {
    result = cps_bind_any(body, cell);
  }
  return result;
}

atom_t
lisp_cps_bind(const atom_t cell)
{
  TRACE_SEXP(cell);
  /*
   * If it's NIL, return.
   */
  if (IS_NULL(cell)) {
    return cell;
  }
  /*
   * Grab CAR and CDR.
   */
  atom_t car = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  /*
   * Call bind recursively.
   */
  atom_t cur = lisp_cps_bind(cdr);
  /*
   * If cur is NIL, just cons with CAR.
   */
  if (IS_NULL(cur)) {
    return car;
  }
  /*
   * Split the lambda and append cur to the body.
   */
  SPLIT_LAMBDA(car, symb, larg, lbdy);
  /*
   * Bind the body to the current continuation.
   */
  atom_t next = cps_bind(lbdy, cur);
  /*
   * And rebuild the lambda.
   */
  MAKE_LAMBDA(result, symb, larg, next);
  return result;
}
