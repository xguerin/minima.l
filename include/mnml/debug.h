#pragma once

#ifdef LISP_ENABLE_DEBUG

#include <mnml/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>

extern bool MNML_DEBUG;
extern bool MNML_VERBOSE_CONS;
extern bool MNML_VERBOSE_RC;
extern bool MNML_VERBOSE_SLOT;
extern bool MNML_VERBOSE_SLAB;

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
