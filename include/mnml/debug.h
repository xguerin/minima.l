#pragma once

#ifdef LISP_ENABLE_DEBUG

#include <mnml/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

/*
 * Debug flags.
 */

extern bool MNML_DEBUG;
extern bool MNML_DEBUG_BIND;
extern bool MNML_DEBUG_CHAN;
extern bool MNML_DEBUG_CLOS;
extern bool MNML_DEBUG_CONS;
extern bool MNML_DEBUG_EVAL;
extern bool MNML_DEBUG_MAKE;
extern bool MNML_DEBUG_MODL;
extern bool MNML_DEBUG_REFC;
extern bool MNML_DEBUG_SLOT;
extern bool MNML_DEBUG_SLAB;
extern bool MNML_DEBUG_TAIL;

void lisp_debug_parse_flags();

/*
 * Debug macros.
 */

#define SEXP_DEBUG_LEVEL 3

#define FPRINTF(__h, __f, ...)                                           \
  if (MNML_DEBUG) {                                                      \
    const int __L = __LINE__;                                            \
    const char* __F = __FUNCTION__;                                      \
    char __b[80];                                                        \
    struct timeval __tv;                                                 \
    gettimeofday(&__tv, NULL);                                           \
    int __u = __tv.tv_usec;                                              \
    strftime(__b, sizeof(__b), "%T", localtime(&__tv.tv_sec));           \
    fprintf(__h, "! %8s.%06d (%-16.16s, %03d) " __f, __b, __u, __F, __L, \
            ##__VA_ARGS__);                                              \
  }

#define ERROR(__fmt, ...)                                         \
  do {                                                            \
    FPRINTF(stderr, "[ERROR ] - { } " __fmt "\n", ##__VA_ARGS__); \
  } while (0)

#define TRACE(__fmt, ...)                                         \
  do {                                                            \
    FPRINTF(stderr, "[TRACE ] - { } " __fmt "\n", ##__VA_ARGS__); \
  } while (0)

#define TRACE_CATG(__k, __fmt, ...) \
  do {                              \
    if (MNML_DEBUG_##__k) {         \
      TRACE(__fmt, ##__VA_ARGS__);  \
    }                               \
  } while (0)

#if defined(__MACH__) || defined(__OpenBSD__)

#define HEADER_SEXP(__c)                                \
  do {                                                  \
    FPRINTF(stderr, "[%06llu] - {%c} %s = ", __c->refs, \
            IS_TAIL_CALL(__c) ? 'T' : ' ', #__c);       \
  } while (0)

#define HEADER_REFC(__f, __t, __n)                                \
  do {                                                            \
    FPRINTF(stderr, "[%2llu->%2llu] - { } %s = ", __f, __t, __n); \
  } while (0)

#define HEADER_SLOT(__i)                            \
  do {                                              \
    FPRINTF(stderr, "[SLOT  ] - { } @%lu = ", __i); \
  } while (0)

#else

#define HEADER_SEXP(__c)                              \
  do {                                                \
    FPRINTF(stderr, "[%06lu] - { } %s = ", __c->refs, \
            IS_TAIL_CALL(__c) ? 'T' : ' ', #__c);     \
  } while (0)

#define HEADER_REFC(__f, __t, __n)                              \
  do {                                                          \
    FPRINTF(stderr, "[%2lu->%2lu] - { } %s = ", __f, __t, __n); \
  } while (0)

#define HEADER_SLOT(__i)                            \
  do {                                              \
    FPRINTF(stderr, "[SLOT  ] - { } @%lu = ", __i); \
  } while (0)

#endif

#define TRACE_BIND(__fmt, ...) TRACE_CATG(BIND, __fmt, ##__VA_ARGS__)
#define TRACE_CHAN(__fmt, ...) TRACE_CATG(CHAN, __fmt, ##__VA_ARGS__)
#define TRACE_CLOS(__fmt, ...) TRACE_CATG(CLOS, __fmt, ##__VA_ARGS__)
#define TRACE_CONS(__fmt, ...) TRACE_CATG(CONS, __fmt, ##__VA_ARGS__)
#define TRACE_EVAL(__fmt, ...) TRACE_CATG(EVAL, __fmt, ##__VA_ARGS__)
#define TRACE_MAKE(__fmt, ...) TRACE_CATG(MAKE, __fmt, ##__VA_ARGS__)
#define TRACE_MODL(__fmt, ...) TRACE_CATG(MODL, __fmt, ##__VA_ARGS__)
#define TRACE_REFC(__fmt, ...) TRACE_CATG(REFC, __fmt, ##__VA_ARGS__)
#define TRACE_SLAB(__fmt, ...) TRACE_CATG(SLAB, __fmt, ##__VA_ARGS__)
#define TRACE_SLOT(__fmt, ...) TRACE_CATG(SLOT, __fmt, ##__VA_ARGS__)
#define TRACE_TAIL(__fmt, ...) TRACE_CATG(TAIL, __fmt, ##__VA_ARGS__)

/*
 * S-expression tracing.
 */

#define TRACE_SEXP(__c)                        \
  do {                                         \
    HEADER_SEXP(__c);                          \
    lisp_debug(stderr, __c, SEXP_DEBUG_LEVEL); \
  } while (0)

#define TRACE_CATG_SEXP(__k, __c) \
  do {                            \
    if (MNML_DEBUG_##__k) {       \
      TRACE_SEXP(__c);            \
    }                             \
  } while (0)

#define TRACE_BIND_SEXP(__c) TRACE_CATG_SEXP(BIND, __c)
#define TRACE_CHAN_SEXP(__c) TRACE_CATG_SEXP(CHAN, __c)
#define TRACE_CLOS_SEXP(__c) TRACE_CATG_SEXP(CLOS, __c)
#define TRACE_CONS_SEXP(__c) TRACE_CATG_SEXP(CONS, __c)
#define TRACE_EVAL_SEXP(__c) TRACE_CATG_SEXP(EVAL, __c)
#define TRACE_MAKE_SEXP(__c) TRACE_CATG_SEXP(MAKE, __c)
#define TRACE_MODL_SEXP(__c) TRACE_CATG_SEXP(MODL, __c)
#define TRACE_SLAB_SEXP(__c) TRACE_CATG_SEXP(SLAB, __c)
#define TRACE_TAIL_SEXP(__c) TRACE_CATG_SEXP(TAIL, __c)

#define TRACE_REFC_SEXP(__f, __t, __n, __c)      \
  do {                                           \
    if (MNML_DEBUG_REFC) {                       \
      HEADER_REFC(__f, __t, __n);                \
      lisp_debug(stderr, __c, SEXP_DEBUG_LEVEL); \
    }                                            \
  } while (0)

#define TRACE_SLOT_SEXP(__i, __c)                \
  do {                                           \
    if (MNML_DEBUG_SLOT) {                       \
      HEADER_SLOT(__i);                          \
      lisp_debug(stderr, __c, SEXP_DEBUG_LEVEL); \
    }                                            \
  } while (0)

void lisp_debug(FILE* fp, const atom_t atom, const size_t level);

#else

#define ERROR(__fmt, ...)

#define TRACE(__fmt, ...)
#define TRACE_BIND(__c, ...)
#define TRACE_CHAN(__c, ...)
#define TRACE_CLOS(__c, ...)
#define TRACE_CONS(__c, ...)
#define TRACE_EVAL(__c, ...)
#define TRACE_MAKE(__c, ...)
#define TRACE_MODL(__c, ...)
#define TRACE_REFC(__c, ...)
#define TRACE_SLAB(__c, ...)
#define TRACE_SLOT(__c, ...)
#define TRACE_TAIL(__c, ...)

#define TRACE_SEXP(__c)
#define TRACE_BIND_SEXP(__c)
#define TRACE_CHAN_SEXP(__c)
#define TRACE_CLOS_SEXP(__c)
#define TRACE_CONS_SEXP(__c)
#define TRACE_EVAL_SEXP(__c)
#define TRACE_MAKE_SEXP(__c)
#define TRACE_MODL_SEXP(__c)
#define TRACE_REFC_SEXP(__c)
#define TRACE_SLAB_SEXP(__c)
#define TRACE_SLOT_SEXP(__c)
#define TRACE_TAIL_SEXP(__c)

#endif

// vim: tw=80:sw=2:ts=2:sts=2:et
