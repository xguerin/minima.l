#pragma once

#ifdef LISP_ENABLE_DEBUG

#include <mnml/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>

/*
 * Debug flags.
 */

extern bool MNML_DEBUG;
extern bool MNML_VERBOSE_BIND;
extern bool MNML_VERBOSE_CHAN;
extern bool MNML_VERBOSE_CONS;
extern bool MNML_VERBOSE_MAKE;
extern bool MNML_VERBOSE_PLUG;
extern bool MNML_VERBOSE_REFC;
extern bool MNML_VERBOSE_SLOT;
extern bool MNML_VERBOSE_SLAB;

void lisp_debug_parse_flags();

/*
 * Debug macros.
 */

#define FPRINTF(__h, __f, ...)                                                          \
  if (MNML_DEBUG) { const int __L = __LINE__;                                           \
    const char * __F = __FUNCTION__;                                                    \
    char __b[80];                                                                       \
    struct timeval __tv;                                                                \
    gettimeofday(&__tv, NULL);                                                          \
    int __u = __tv.tv_usec;                                                             \
    strftime(__b, sizeof(__b), "%T", localtime(&__tv.tv_sec));                          \
    fprintf(__h, "! %8s.%06d (%-16.16s, %03d) " __f, __b, __u,  __F, __L, __VA_ARGS__); \
  }

#define ERROR(__fmt, ...) \
  FPRINTF(stderr, "[ERROR ] - " __fmt "\n", __VA_ARGS__)

#define TRACE(__fmt, ...) \
  FPRINTF(stderr, "[TRACE ] - " __fmt "\n", __VA_ARGS__)

#ifdef __MACH__

#define HEADER_SEXP(__c)  \
  FPRINTF(stderr, "[%06llu] - %s = ", __c->refs, #__c)

#else

#define HEADER_SEXP(__c)  \
  FPRINTF(stderr, "[%06lu] - %s = ", __c->refs, #__c)

#endif

#define TRACE_SEXP(__c) {   \
  HEADER_SEXP(__c);         \
  lisp_debug(stderr, __c);  \
}

#define TRACE_CATG_SEXP(__k, __c) { \
  if (__k) {                        \
    TRACE_SEXP(__c)                 \
  }                                 \
}

#define TRACE_BIND_SEXP(__c) TRACE_CATG_SEXP(MNML_VERBOSE_BIND, __c)
#define TRACE_CHAN_SEXP(__c) TRACE_CATG_SEXP(MNML_VERBOSE_CHAN, __c)
#define TRACE_CONS_SEXP(__c) TRACE_CATG_SEXP(MNML_VERBOSE_CONS, __c)
#define TRACE_PLUG_SEXP(__c) TRACE_CATG_SEXP(MNML_VERBOSE_PLUG, __c)
#define TRACE_SLAB_SEXP(__c) TRACE_CATG_SEXP(MNML_VERBOSE_SLAB, __c)

void lisp_debug(FILE * fp, const atom_t atom);

#else

#define TRACE(__fmt, ...)
#define TRACE_SEXP(__c)
#define TRACE_BIND_SEXP(__c)
#define TRACE_CHAN_SEXP(__c)
#define TRACE_CONS_SEXP(__c)
#define TRACE_PLUG_SEXP(__c)

#endif
