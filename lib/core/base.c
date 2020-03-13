#include <mnml/debug.h>
#include <mnml/lexer.h>
#include <mnml/lisp.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * List context functions.
 */

lisp_t
lisp_new(const atom_t ichan, const atom_t ochan)
{
  lisp_t lisp = (lisp_t)malloc(sizeof(struct _lisp));
  lisp->globals = UP(NIL);
  lisp->ichan = UP(ichan);
  lisp->ochan = UP(ochan);
  return lisp;
}

void
lisp_delete(lisp_t lisp)
{
  X(lisp->ochan);
  X(lisp->ichan);
  X(lisp->globals);
  free(lisp);
}

/*
 * Global symbols.
 */

atom_t NIL = NULL;
atom_t TRUE = NULL;
atom_t QUOTE = NULL;
atom_t WILDCARD = NULL;

/*
 * Symbol lookup.
 */

atom_t
lisp_lookup(const lisp_t lisp, const atom_t closure, const symbol_t sym)
{
  /*
   * Look for the symbol the closure.
   */
  FOREACH(closure, a)
  {
    atom_t car = a->car;
    if (lisp_symbol_match(CAR(car), sym)) {
      return UP(CDR(car));
    }
    NEXT(a);
  }
  /*
   * Check the global environment.
   */
  FOREACH(GLOBALS, g)
  {
    atom_t car = g->car;
    if (lisp_symbol_match(CAR(car), sym)) {
      return UP(CDR(car));
    }
    NEXT(g);
  }
  /*
   * Nothing found.
   */
  return UP(NIL);
}

/*
 * Basic functions.
 */

atom_t
lisp_car(const atom_t cell)
{
  if (likely(IS_PAIR(cell))) {
    return UP(CAR(cell));
  }
  return UP(NIL);
}

atom_t
lisp_cdr(const atom_t cell)
{
  if (likely(IS_PAIR(cell))) {
    return UP(CDR(cell));
  }
  return UP(NIL);
}

/*
 * Internal list construction functions.
 */

atom_t
lisp_cons(const atom_t car, const atom_t cdr)
{
  atom_t R = lisp_allocate();
  R->type = T_PAIR;
  R->refs = 1;
  CAR(R) = UP(car);
  CDR(R) = UP(cdr);
  TRACE_CONS_SEXP(R);
  return R;
}

atom_t
lisp_conc(const atom_t car, const atom_t cdr)
{
  atom_t R;
  /*
   */
  if (likely(IS_PAIR(car))) {
    FOREACH(car, p) {
      NEXT(p);
    }
    X(p->cdr);
    p->cdr = UP(cdr);
    R = UP(car);
  } else {
    R = UP(cdr);
  }
  /*
   */
  TRACE_CONS_SEXP(R);
  return R;
}

/*
 * Lisp READ.
 */

#define RBUFLEN 1024

static void
lisp_consumer(const lisp_t lisp, const atom_t cell)
{
  atom_t chn = CAR(ICHAN);
  CDR(chn) = lisp_append(CDR(chn), cell);
}

static atom_t
lisp_read_pop(const lisp_t lisp)
{
  TRACE_CHAN_SEXP(ICHAN);
  /*
   * ((CHN0 PWD V1 V2) (CHN1 PWD V1 V2) ...).
   */
  atom_t chn = CAR(ICHAN);
  atom_t vls = CDR(CDR(chn));
  atom_t res = UP(CAR(vls));
  CDR(CDR(chn)) = UP(CDR(vls));
  X(vls);
  /*
   */
  TRACE_CHAN_SEXP(ICHAN);
  return res;
}

atom_t
lisp_read(const lisp_t lisp, UNUSED const atom_t closure, const atom_t cell)
{
  TRACE_CHAN_SEXP(ICHAN);
  X(cell);
  /*
   * Grab the channel, the path and the content.
   */
  atom_t chn = CAR(ICHAN);
  atom_t hnd = CAR(chn);
  atom_t val = CDR(CDR(chn));
  /*
   * Check if there is any value in the channel's buffer.
   */
  if (!IS_NULL(val)) {
    return lisp_read_pop(lisp);
  }
  /*
   * Read from the file descriptor.
   */
  lexer_t lexer = lexer_create(lisp, lisp_consumer);
  FILE* handle = (FILE*)hnd->number;
  /*
   */
  char buffer[RBUFLEN] = { 0 };
  do {
    char* p = fgets(buffer + lexer->rem, RBUFLEN - lexer->rem, handle);
    if (p == NULL) {
      break;
    }
    size_t len = strlen(p);
    lexer_parse(lexer, buffer, lexer->rem + len, lexer->rem + len < RBUFLEN);
  } while (IS_NULL(CDR(CDR(CAR(ICHAN)))) || lexer_pending(lexer));
  /*
   * Grab the result and return it.
   */
  lexer_destroy(lexer);
  return IS_NULL(CDR(CDR(chn))) ? NULL : lisp_read_pop(lisp);
}

/*
 * SETQ. PAIR is consumed.
 */

atom_t
lisp_setq(const atom_t closure, const atom_t pair)
{
  /*
   * Check if pair is valid.
   */
  if (!IS_PAIR(pair) || !IS_SYMB(CAR(pair))) {
    X(pair);
    return UP(closure);
  }
  /*
   * If the closure is NIL, return the wrapped pair.
   */
  if (IS_NULL(closure)) {
    atom_t res = lisp_cons(pair, closure);
    X(pair);
    return res;
  }
  /*
   * Extract CAR and CDR.
   */
  atom_t car = lisp_car(closure);
  atom_t cdr = lisp_cdr(closure);
  /*
   * Replace car if its symbol matches pair's.
   */
  if (lisp_symbol_match(CAR(car), &CAR(pair)->symbol)) {
    atom_t res = lisp_cons(pair, cdr);
    X(pair, cdr, car);
    return res;
  }
  /*
   * Look further down the closure.
   */
  atom_t nxt = lisp_setq(cdr, pair);
  atom_t res = lisp_cons(car, nxt);
  X(cdr, car, nxt);
  return res;
}

/*
 * PROG. CELL and RESULT are consumed.
 */

atom_t
lisp_prog(const lisp_t lisp, const atom_t closure, const atom_t cell,
          const atom_t result)
{
  if (likely(IS_PAIR(cell))) {
    /*
     * Get CAR/CDR.
     */
    atom_t res = lisp_eval(lisp, closure, lisp_car(cell));
    atom_t cdr = lisp_cdr(cell);
    /*
     */
    X(cell, result);
    return lisp_prog(lisp, closure, cdr, res);
  }
  /*
   */
  X(cell);
  return result;
}

/*
 * Print function.
 */

#define IO_BUFFER_LEN 1024

static size_t lisp_prin_atom(FILE* const handle, char* const buf,
                             const size_t idx, const atom_t closure,
                             const atom_t cell, const bool s);

static size_t
lisp_write(FILE* const handle, char* const buf, const size_t idx, void* data,
           const size_t len)
{
  size_t pidx = idx;
  /*
   * Flush the buffer if necessary.
   */
  if (pidx + len >= IO_BUFFER_LEN) {
    fwrite(buf, 1, pidx, handle);
    pidx = 0;
  }
  /*
   * Append the new data.
   */
  memcpy(&buf[pidx], data, len);
  return pidx + len;
}

static void
lisp_flush(FILE* const handle, char* const buf, const size_t idx)
{
  fwrite(buf, 1, idx, handle);
  fflush(handle);
}

static size_t
lisp_prin_pair(FILE* const handle, char* const buf, const size_t idx,
               const atom_t closure, const atom_t cell, const bool s)
{
  size_t nxt = 0;
  /*
   * Print CAR.
   */
  nxt = lisp_prin_atom(handle, buf, idx, closure, CAR(cell), s);
  /*
   * Print CDR.
   */
  if (!IS_NULL(CDR(cell))) {
    if (IS_PAIR(CDR(cell))) {
      if (s)
        nxt = lisp_write(handle, buf, nxt, " ", 1);
      return lisp_prin_pair(handle, buf, nxt, closure, CDR(cell), s);
    } else {
      if (s)
        nxt = lisp_write(handle, buf, nxt, " . ", 3);
      return lisp_prin_atom(handle, buf, nxt, closure, CDR(cell), s);
    }
  }
  /*
   */
  return nxt;
}

static size_t
lisp_prin_atom(FILE* const handle, char* const buf, const size_t idx,
               const atom_t closure, const atom_t cell, const bool s)
{
  switch (cell->type) {
    case T_NIL:
      return s ? lisp_write(handle, buf, idx, "NIL", 3) : 0;
    case T_TRUE:
      return lisp_write(handle, buf, idx, "T", 1);
    case T_CHAR: {
      char c = (char)cell->number;
      if (s) {
        size_t nxt = lisp_write(handle, buf, idx, "^", 1);
        switch (c) {
          case '\033':
            nxt = lisp_write(handle, buf, nxt, "\\e", 2);
            break;
          case '\n':
            nxt = lisp_write(handle, buf, nxt, "\\n", 2);
            break;
          case '\r':
            nxt = lisp_write(handle, buf, nxt, "\\r", 2);
            break;
          case '\t':
            nxt = lisp_write(handle, buf, nxt, "\\t", 2);
            break;
          default:
            nxt = lisp_write(handle, buf, nxt, &c, 1);
            break;
        }
        return nxt;
      }
      return lisp_write(handle, buf, idx, &c, 1);
    }
    case T_PAIR: {
      size_t nxt = idx;
      if (s)
        nxt = lisp_write(handle, buf, nxt, "(", 1);
      nxt = lisp_prin_pair(handle, buf, nxt, closure, cell, s);
      if (s)
        nxt = lisp_write(handle, buf, nxt, ")", 1);
      return nxt;
    }
    case T_NUMBER: {
      char buffer[24] = { 0 };
#if defined(__MACH__) || defined(__OpenBSD__)
      sprintf(buffer, "%lld", cell->number);
#else
      sprintf(buffer, "%ld", cell->number);
#endif
      return lisp_write(handle, buf, idx, buffer, strlen(buffer));
    }
    case T_SYMBOL:
      return lisp_write(handle, buf, idx, cell->symbol.val,
                        strnlen(cell->symbol.val, LISP_SYMBOL_LENGTH));
    case T_WILDCARD:
      return lisp_write(handle, buf, idx, "_", 1);
    default:
      return 0;
  }
}

void
lisp_prin(const lisp_t lisp, const atom_t closure, const atom_t cell,
          const bool s)
{
  FILE* handle = (FILE*)CAR(CAR(OCHAN))->number;
  char buffer[IO_BUFFER_LEN];
  size_t idx = lisp_prin_atom(handle, buffer, 0, closure, cell, s);
  lisp_flush(handle, buffer, idx);
}

// vim: tw=80:sw=2:ts=2:sts=2:et
