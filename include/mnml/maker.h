#pragma once

#include <mnml/types.h>

atom_t lisp_make_char(const char c);
atom_t lisp_make_number(const int64_t num);
atom_t lisp_make_string(const char * const s, const size_t len);
atom_t lisp_make_symbol(const symbol_t sym);
