#pragma once

#include <mnml/lisp.h>
#include <stdlib.h>

/*
 * Make a C string from a list of characters.
 */
size_t lisp_make_string(const atom_t cell, char * const buffer,
                        const size_t len, const size_t idx);

/*
 * Process escapes in a list of characters.
 */
atom_t lisp_process_escapes(const atom_t cell, const bool esc, const atom_t res);

/*
 * Get a timestamp in nanoseconds.
 */
uint64_t lisp_timestamp();
