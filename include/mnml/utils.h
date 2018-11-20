#pragma once

#include <mnml/lisp.h>
#include <stdlib.h>

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
