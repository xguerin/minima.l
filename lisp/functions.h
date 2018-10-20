#pragma once

#include "lisp.h"

typedef cell_t (* function_t)(const cell_t cell);

cell_t lisp_function_quote(const cell_t cell);

#define IS_EVAL(__f) (__f != (uintptr_t)lisp_function_quote)

void lisp_function_register_all();
