#pragma once

#include <mnml/types.h>

/*
 * Continuation-passing style functions.
 */

atom_t lisp_cps_convert(const atom_t cell);
atom_t lisp_cps_bind(const atom_t cell);
