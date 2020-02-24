#pragma once

#include <mnml/module.h>
#include <stdbool.h>

extern atom_t MODULES;

/*
 * Module search.
 */

bool lisp_module_find(const char* const paths, const atom_t sym,
                      char* const path);

/*
 * Module load.
 */

atom_t lisp_module_load_binary(const char* const path, const lisp_t lisp,
                               const atom_t name, const atom_t symbols);

atom_t lisp_module_load_script(const char* const path, const lisp_t lisp,
                               const atom_t symbols);
