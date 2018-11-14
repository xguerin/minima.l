#include "lisp.h"
#include "slab.h"
#include "utils.h"
#include <limits.h>
#include <time.h>

size_t
lisp_make_string(const atom_t cell, char * const buffer, const size_t idx)
{
  /*
   * Terminate the string.
   */
  if (IS_NULL(cell) || idx == PATH_MAX) {
    *buffer = '\0';
    return idx;
  }
  /*
   * Process the chars.
   */
  size_t res = lisp_make_string(CDR(cell), buffer + 1, idx + 1);
  *buffer = CAR(cell)->number;
  return res;
}

atom_t
lisp_process_escapes(const atom_t cell, const bool esc, const atom_t res)
{
  TRACE_SEXP(cell);
  bool nesc = false;
  /*
   */
  if (cell == NIL) {
    X(cell);
    TRACE_SEXP(res);
    return res;
  }
  /*
   */
  atom_t nxt;
  atom_t car = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  /*
   * Process the character.
   */
  if (esc) {
    switch ((char)car->number) {
      case 'n' :
        X(car);
        car = lisp_make_char('\n');
        break;
      case 't' :
        X(car);
        car = lisp_make_char('\t');
        break;
      default:
        break;
    }
    atom_t con = lisp_cons(car, NIL);
    nxt = lisp_conc(res, con);
    X(res); X(con);
    nesc = false;
  }
  else if (car->number == '\\') {
    nesc = true;
    nxt = res;
  }
  else {
    atom_t con = lisp_cons(car, NIL);
    nxt = lisp_conc(res, con);
    X(res); X(con);
  }
  /*
   */
  X(car);
  return lisp_process_escapes(cdr, nesc, nxt);
}

uint64_t
lisp_timestamp()
{
  struct timespec ts = { 0 };
  clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
  return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}
