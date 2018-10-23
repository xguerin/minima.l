#pragma once

#include "lisp.h"

/*
 * Functiom type.
 */

typedef cell_t (* function_t)(const cell_t closure, const cell_t cell);

/*
 * Function helpers.
 */

void lisp_function_register_all();
