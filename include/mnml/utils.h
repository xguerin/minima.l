#pragma once

#include <mnml/maker.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/*
 * Interpreter life cycle.
 */

typedef void (* error_handler_t)();

void lisp_set_parse_error_handler(const error_handler_t h);
void lisp_set_syntax_error_handler(const error_handler_t h);

const char * lisp_prefix();

void lisp_init();
void lisp_fini();

/*
 * Return the length of a list.
 */

size_t lisp_len(const atom_t cell);

/*
 * Destructively append element ELT to list LST.
 */
atom_t lisp_append(const atom_t lst, const atom_t elt);

/*
 * Shallow duplicate: 1(1X 1X ...) -> 1(2X 2X ...).
 */
atom_t lisp_dup(const atom_t cell);

/*
 * Merge define-site and call-site closures. The call-site closure is consumed.
 */
atom_t lisp_merge(const atom_t defn, const atom_t call);

/*
 * Make a C string from a list of characters.
 */
size_t lisp_make_cstring(const atom_t cell, char * const buffer,
                         const size_t len, const size_t idx);

/*
 * Process escapes in a list of characters.
 */
atom_t lisp_process_escapes(const atom_t cell, const bool esc, const atom_t res);

/*
 * Get a timestamp in nanoseconds.
 */
uint64_t lisp_timestamp();

/*
 * IO context helpers.
 */
#define PUSH_IO_CONTEXT(__c, __d) {           \
  atom_t n = lisp_make_number((int64_t)__d);  \
  atom_t l = lisp_cons(n, NIL);               \
  atom_t o = __c;                             \
  __c = lisp_cons(l, o);                      \
  X(o); X(l); X(n);                           \
}

#define POP_IO_CONTEXT(__c) { \
  atom_t old = __c;           \
  __c = UP(CDR(__c));         \
  X(old);                     \
}

/*
 * Symbol matching.
 */

static inline bool
lisp_symbol_match(const atom_t a, const atom_t b)
{
  register __m128i res = _mm_xor_si128(a->symbol.tag, b->symbol.tag);
  return _mm_test_all_zeros(res, res);
}
