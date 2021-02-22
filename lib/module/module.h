#pragma once

#include <mnml/module.h>
#include <stdbool.h>

/*
 * Module search.
 */

bool module_find(const char* const paths, const atom_t sym, char* const path);

/*
 * Module load.
 */

atom_t module_load_binary(const char* const path, const lisp_t lisp,
                          const atom_t name, const atom_t symbols);
