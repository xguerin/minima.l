#pragma once

#include <mnml/types.h>

/*
 * Continuation-passing style functions.
 */

atom_t lisp_cps_lift(const atom_t cell, const size_t padding);
atom_t lisp_cps_bind(const atom_t cell);
