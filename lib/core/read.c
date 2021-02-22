#include <mnml/debug.h>
#include <mnml/lexer.h>
#include <mnml/lisp.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RBUFLEN 1024

static void
lisp_consumer(const lisp_t lisp, const atom_t cell)
{
  atom_t chn = CAR(ICHAN);
  CDR(chn) = lisp_append(lisp, CDR(chn), cell);
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
  X(lisp->slab, vls);
  /*
   */
  TRACE_CHAN_SEXP(ICHAN);
  return res;
}

atom_t
lisp_read(const lisp_t lisp, UNUSED const atom_t closure, const atom_t cell)
{
  TRACE_CHAN_SEXP(ICHAN);
  X(lisp->slab, cell);
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

// vim: tw=80:sw=2:ts=2:sts=2:et
