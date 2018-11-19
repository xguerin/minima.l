#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <limits.h>
#include <time.h>

atom_t
lisp_append(const atom_t lst, const atom_t elt)
{
  atom_t con = lisp_cons(elt, NIL);
  atom_t res = lisp_conc(lst, con);
  X(con); X(elt); X(lst);
  TRACE_CONS(res);
  return res;
}

size_t
lisp_make_cstring(const atom_t cell, char * const buffer,
                  const size_t len, const size_t idx)
{
  /*
   * Terminate the string.
   */
  if (IS_NULL(cell) || idx == len) {
    *buffer = '\0';
    return idx;
  }
  /*
   * Process the chars.
   */
  size_t res = lisp_make_cstring(CDR(cell), buffer + 1, len, idx + 1);
  *buffer = CAR(cell)->number;
  return res;
}

atom_t
lisp_process_escapes(const atom_t cell, const bool esc, const atom_t res)
{
  bool nesc = false;
  /*
   */
  if (cell == NIL) {
    X(cell);
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
    nxt = lisp_append(res, car);
    nesc = false;
  }
  else if (car->number == '\\') {
    X(car);
    nesc = true;
    nxt = res;
  }
  else {
    nxt = lisp_append(res, car);
  }
  /*
   */
  return lisp_process_escapes(cdr, nesc, nxt);
}

uint64_t
lisp_timestamp()
{
  struct timespec ts = { 0 };
  clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
  return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}
