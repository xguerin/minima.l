#pragma once

#include "types.h"
#include <stdio.h>

#ifdef LISP_ENABLE_DEBUG

#define FPRINTF \
  if (getenv("MNML_DEBUG")) fprintf

#define TRACE(__fmt, ...) \
  FPRINTF(stderr, "! %s:%d: " __fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__)

#ifdef __MACH__

#define HEADER_SEXP(__c)  \
  FPRINTF(stderr, "! %s:%d: [%llu] %s = ", __FUNCTION__, __LINE__, __c->refs, #__c)

#else

#define HEADER_SEXP(__c)  \
  FPRINTF(stderr, "! %s:%d: [%lu] %s = ", __FUNCTION__, __LINE__, __c->refs, #__c)

#endif

#define TRACE_SEXP(__c) {   \
  HEADER_SEXP(__c);         \
  lisp_debug(stderr, __c);  \
}

void lisp_debug(FILE * fp, const atom_t atom);

#else

#define TRACE(__fmt, ...)
#define TRACE_SEXP(__c)

#endif
