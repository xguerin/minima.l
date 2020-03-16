#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <errno.h>
#include <sys/select.h>
#include <unistd.h>

static int
convert(const atom_t fds, fd_set* const rset, fd_set* const eset)
{
  int cur = 0;
  /*
   * Base case.
   */
  if (IS_NULL(fds) || !IS_PAIR(fds)) {
    return -1;
  }
  /*
   * Add CAR in the result set if valid.
   */
  if (IS_NUMB(CAR(fds))) {
    if (CAR(fds)->number >= 0 && CAR(fds)->number < UINT32_MAX) {
      FD_SET(CAR(fds)->number, rset);
      FD_SET(CAR(fds)->number, eset);
      cur = CAR(fds)->number;
    }
  }
  /*
   * Process the rest of the list.
   */
  int nxt = convert(CDR(fds), rset, eset);
  return cur > nxt ? cur : nxt;
}

static atom_t
process_r(const lisp_t lisp, const atom_t closure, const atom_t fds,
          const atom_t cb, const fd_set* const set)
{
  atom_t act = UP(NIL);
  /*
   * Base case.
   */
  if (IS_NULL(fds) || !IS_PAIR(fds)) {
    return act;
  }
  /*
   * Check if CAR is in the set.
   */
  if (IS_NUMB(CAR(fds))) {
    /*
     * Keep the file descriptor around.
     */
    X(act);
    act = UP(TRUE);
    /*
     * Call the callback if it is set.
     */
    if (FD_ISSET(CAR(fds)->number, set) && !IS_NULL(cb)) {
      atom_t fdn = lisp_car(fds);
      atom_t cn0 = lisp_cons(fdn, NIL);
      atom_t cn1 = lisp_cons(cb, cn0);
      X(act, fdn, cn0);
      act = lisp_eval(lisp, closure, cn1);
    }
  }
  /*
   * Process the action.
   */
  switch (act->type) {
    case T_NUMBER: {
      /*
       * Append the head of the set.
       */
      atom_t car = lisp_car(fds);
      atom_t nxt = process_r(lisp, closure, CDR(fds), cb, set);
      atom_t cn0 = lisp_cons(car, nxt);
      X(car, nxt);
      /*
       * If the returned number is different, append it too.
       */
      if (car->number != act->number) {
        atom_t tmp = lisp_cons(act, cn0);
        X(cn0);
        cn0 = tmp;
      }
      /*
       * Return the new set.
       */
      X(act);
      return cn0;
    }
    case T_NIL: {
      X(act);
      close(CAR(fds)->number);
      return process_r(lisp, closure, CDR(fds), cb, set);
    }
    default: {
      atom_t car = lisp_car(fds);
      atom_t nxt = process_r(lisp, closure, CDR(fds), cb, set);
      atom_t cn0 = lisp_cons(car, nxt);
      X(act, car, nxt);
      return cn0;
    }
  }
}

static atom_t
process(const lisp_t lisp, const atom_t closure, const atom_t fds,
        const char* const cb_name, const fd_set* const set)
{
  MAKE_SYMBOL_STATIC(cb_s, cb_name, strlen(cb_name));
  atom_t cbk = lisp_make_symbol(cb_s);
  atom_t res = process_r(lisp, closure, fds, cbk, set);
  X(cbk);
  return res;
}

static atom_t USED
lisp_function_select(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, fds, closure, FDS);
  /*
   * Check that the fds argument is a list.
   */
  if (!IS_LIST(fds)) {
    X(fds);
    return UP(NIL);
  }
  /*
   * Initialize the READ and ERROR sets.
   */
  fd_set rset, eset;
  FD_ZERO(&rset);
  FD_ZERO(&eset);
  int fdmax = convert(fds, &rset, &eset);
  /*
   * Call select();
   */
  int res = select(fdmax + 1, &rset, NULL, &eset, NULL);
  if (res < 0) {
    X(fds);
    TRACE("select() failed: %s", strerror(errno));
    return UP(NIL);
  }
  /*
   * Process the events.
   */
  atom_t rres = process(lisp, closure, fds, "ON_READ", &rset);
  atom_t eres = process(lisp, closure, fds, "ON_ERROR", &eset);
  /*
   * Build the result.
   */
  atom_t cn0 = lisp_cons(eres, NIL);
  atom_t cn1 = lisp_cons(rres, cn0);
  X(rres, cn0, eres);
  /*
   * Return the updated descriptor list.
   */
  X(fds);
  return cn1;
}

LISP_MODULE_SETUP(select, select, FDS, ON_READ, ON_ERROR, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
