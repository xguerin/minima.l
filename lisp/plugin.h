#pragma once

#include "lisp.h"
#include <stdbool.h>

/*
 * Plugin management.
 */

atom_t lisp_plugin_load(const atom_t sym);