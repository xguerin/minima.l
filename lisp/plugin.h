#pragma once

#include "lisp.h"
#include <stdbool.h>

/*
 * Plugin management.
 */

atom_t lisp_plugin_load(const char * const sym);

/*
 * Debug.
 */

#ifdef LISP_ENABLE_DEBUG

#define TRACE_PLUGIN(__p) {                             \
  printf("! %s:%d: %s\n", __FUNCTION__, __LINE__, __p); \
}

#else

#define TRACE_PLUGIN(__p)

#endif
