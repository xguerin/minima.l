#pragma once

#include "lisp.h"

/*
 * Functiom type.
 */

typedef atom_t (* function_t)(const atom_t closure, const atom_t cell);

/*
 * Function helpers.
 */

void lisp_function_register_all();
