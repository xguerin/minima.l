#include <mnml/lisp.h>
#include <mnml/utils.h>
#include <limits.h>
#include <unistd.h>

/*
 * Atom makers.
 */

extern atom_t lisp_make_char(const lisp_t lisp, const char c);
extern atom_t lisp_make_number(const lisp_t lisp, const int64_t num);
extern atom_t lisp_make_nil(const lisp_t lisp);
extern atom_t lisp_make_true(const lisp_t lisp);
extern atom_t lisp_make_symbol(const lisp_t lisp, const symbol_t sym);
extern atom_t lisp_make_wildcard(const lisp_t lisp);
extern atom_t lisp_make_quote(const lisp_t lisp);

/*
 * FFI-specific interface.
 */

void
lisp_io_push(const lisp_t lisp)
{
  char cwd_buf[PATH_MAX];
  const char* const cwd = getcwd(cwd_buf, PATH_MAX);
  PUSH_IO_CONTEXT(lisp, lisp->ichan, stdin, cwd);
  PUSH_IO_CONTEXT(lisp, lisp->ochan, stdout, cwd);
}

void
lisp_io_pop(const lisp_t lisp)
{
  POP_IO_CONTEXT(lisp, lisp->ichan);
  POP_IO_CONTEXT(lisp, lisp->ochan);
}

extern atom_t lisp_make_symbol_from_string(const lisp_t lisp,
                                           const char* const str,
                                           const size_t len);
extern int lisp_get_type(const atom_t atom);
extern char lisp_get_char(const atom_t atom);
extern int64_t lisp_get_number(const atom_t atom);
extern const char* lisp_get_symbol(const atom_t atom);
extern void lisp_drop(const lisp_t lisp, const atom_t atom);

/*
 * Lisp basic functions.
 */

extern atom_t lisp_car(const lisp_t lisp, const atom_t cell);
extern atom_t lisp_cdr(const lisp_t lisp, const atom_t cell);

/*
 * Internal list construction functions.
 */

extern atom_t lisp_cons(const lisp_t lisp, const atom_t car, const atom_t cdr);
