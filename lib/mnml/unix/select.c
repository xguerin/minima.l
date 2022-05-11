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
  atom_t act = lisp_make_nil(lisp);
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
    X(lisp, act);
    act = lisp_make_true(lisp);
    /*
     * Call the callback if it is set.
     */
    if (FD_ISSET(CAR(fds)->number, set) && !IS_NULL(cb)) {
      atom_t fdn = lisp_car(lisp, fds);
      atom_t cn0 = lisp_cons(lisp, fdn, lisp_make_nil(lisp));
      atom_t cn1 = lisp_cons(lisp, UP(cb), cn0);
      X(lisp, act);
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
      atom_t car = lisp_car(lisp, fds);
      atom_t nxt = process_r(lisp, closure, CDR(fds), cb, set);
      atom_t cn0 = lisp_cons(lisp, car, nxt);
      /*
       * If the returned number is different, append it too.
       */
      if (car->number != act->number) {
        cn0 = lisp_cons(lisp, UP(act), cn0);
      }
      /*
       * Return the new set.
       */
      X(lisp, act);
      return cn0;
    }
    case T_NIL: {
      X(lisp, act);
      close(CAR(fds)->number);
      return process_r(lisp, closure, CDR(fds), cb, set);
    }
    default: {
      X(lisp, act);
      atom_t car = lisp_car(lisp, fds);
      atom_t nxt = process_r(lisp, closure, CDR(fds), cb, set);
      return lisp_cons(lisp, car, nxt);
    }
  }
}

static atom_t
process(const lisp_t lisp, const atom_t closure, const atom_t fds,
        const char* const cb_name, const fd_set* const set)
{
  MAKE_SYMBOL_STATIC(cb_s, cb_name);
  atom_t cbk = lisp_make_symbol(lisp, cb_s);
  atom_t res = process_r(lisp, closure, fds, cbk, set);
  X(lisp, cbk);
  return res;
}

static atom_t USED
lisp_function_select(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, FDS);
  /*
   * Check that the fds argument is a list.
   */
  if (!IS_LIST(FDS)) {
    return lisp_make_nil(lisp);
  }
  /*
   * Initialize the READ and ERROR sets.
   */
  fd_set rset, eset;
  FD_ZERO(&rset);
  FD_ZERO(&eset);
  int fdmax = convert(FDS, &rset, &eset);
  /*
   * Call select();
   */
  int res = select(fdmax + 1, &rset, NULL, &eset, NULL);
  if (res < 0) {
    TRACE("select() failed: %s", strerror(errno));
    return lisp_make_nil(lisp);
  }
  /*
   * Process the events.
   */
  atom_t rres = process(lisp, C, FDS, "ON_READ", &rset);
  atom_t eres = process(lisp, C, FDS, "ON_ERROR", &eset);
  /*
   * Build the result.
   */
  atom_t cn0 = lisp_cons(lisp, eres, lisp_make_nil(lisp));
  atom_t cn1 = lisp_cons(lisp, rres, cn0);
  /*
   * Return the updated descriptor list.
   */
  return cn1;
}

LISP_MODULE_SETUP(select, select, FDS, ON_READ, ON_ERROR, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
