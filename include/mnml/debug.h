#pragma once

#include <mnml/types.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef LISP_ENABLE_DEBUG

extern bool MNML_DEBUG;
extern bool MNML_VERBOSE_CONS;
extern bool MNML_VERBOSE_RC;
extern bool MNML_VERBOSE_SLOT;
extern bool MNML_VERBOSE_SLAB;

#define FPRINTF \
  if (MNML_DEBUG) fprintf

#define ERROR(__fmt, ...) \
  FPRINTF(stderr, "! (%-16.16s, %03d) [ERROR ] - " __fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__)

#define TRACE(__fmt, ...) \
  FPRINTF(stderr, "! (%-16.16s, %03d) [TRACE ] - " __fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__)

#ifdef __MACH__

#define HEADER_SEXP(__c)  \
  FPRINTF(stderr, "! (%-16.16s, %03d) [%06llu] - %s = ", __FUNCTION__, __LINE__, __c->refs, #__c)

#else

#define HEADER_SEXP(__c)  \
  FPRINTF(stderr, "! (%-16.16s, %03d) [%06lu] - %s = ", __FUNCTION__, __LINE__, __c->refs, #__c)

#endif

#define TRACE_SEXP(__c) {   \
  HEADER_SEXP(__c);         \
  lisp_debug(stderr, __c);  \
}

#define TRACE_CONS(__c) {   \
  if (MNML_VERBOSE_CONS) {  \
    TRACE_SEXP(__c)         \
  }                         \
}

void lisp_debug(FILE * fp, const atom_t atom);

#else

#define TRACE(__fmt, ...)
#define TRACE_SEXP(__c)
#define TRACE_CONS(__c)

#endif
