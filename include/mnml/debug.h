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

#define FPRINTF(__h, __f, ...)                                                              \
  if (MNML_DEBUG) { const int __L = __LINE__;                                               \
    const char * __F = __FUNCTION__;                                                        \
    char __b[80];                                                                           \
    struct timeval __tv;                                                                    \
    gettimeofday(&__tv, NULL);                                                              \
    int __u = __tv.tv_usec;                                                                 \
    strftime(__b, sizeof(__b), "%T", localtime(&__tv.tv_sec));                              \
    fprintf(__h, "! %8s.%06d (%-16.16s, %03d) " __f, __b, __u,  __F, __L, ## __VA_ARGS__);  \
  }

#define ERROR(__fmt, ...)                                     \
{                                                             \
  FPRINTF(stderr, "[ERROR ] - " __fmt "\n", ## __VA_ARGS__);  \
}

#define TRACE(__fmt, ...)                                     \
{                                                             \
  FPRINTF(stderr, "[TRACE ] - " __fmt "\n", ## __VA_ARGS__);  \
}

#define TRACE_CATG(__k, __fmt, ...)   \
{                                     \
  if (__k) {                          \
    TRACE(__fmt, ## __VA_ARGS__);    \
  }                                   \
}

#if defined(__MACH__) || defined(__OpenBSD__)

#define HEADER_SEXP(__c)                                \
{                                                       \
  FPRINTF(stderr, "[%06llu] - %s = ", __c->refs, #__c); \
}

#define HEADER_REFC(__f, __t, __n)                      \
{                                                       \
  TRACE_REFS("[%2llu->%2llu] - %s = ", __f, __t, __n);  \
}

#define HEADER_SLOT(__i)                                \
{                                                       \
  TRACE_SLOT("[SLOT  ] - @%lu = ", __i);                \
}

#else

#define HEADER_SEXP(__c)                                \
{                                                       \
  FPRINTF(stderr, "[%06lu] - %s = ", __c->refs, #__c);  \
}

#define HEADER_REFC(__f, __t, __n)                        \
{                                                         \
  FPRINTF(stderr, "[%2lu->%2lu] - %s = ", __f, __t, __n); \
}

#define HEADER_SLOT(__i)                  \
{                                         \
  TRACE_SLOT("[SLOT  ] - @%lu = ", __i);  \
}

#endif

#define TRACE_BIND(__fmt, ...) TRACE_CATG(MNML_VERBOSE_BIND, __fmt, ## __VA_ARGS__)
#define TRACE_CHAN(__fmt, ...) TRACE_CATG(MNML_VERBOSE_CHAN, __fmt, ## __VA_ARGS__)
#define TRACE_CONS(__fmt, ...) TRACE_CATG(MNML_VERBOSE_CONS, __fmt, ## __VA_ARGS__)
#define TRACE_MAKE(__fmt, ...) TRACE_CATG(MNML_VERBOSE_MAKE, __fmt, ## __VA_ARGS__)
#define TRACE_PLUG(__fmt, ...) TRACE_CATG(MNML_VERBOSE_PLUG, __fmt, ## __VA_ARGS__)
#define TRACE_REFC(__fmt, ...) TRACE_CATG(MNML_VERBOSE_REFC, __fmt, ## __VA_ARGS__)
#define TRACE_SLAB(__fmt, ...) TRACE_CATG(MNML_VERBOSE_SLAB, __fmt, ## __VA_ARGS__)
#define TRACE_SLOT(__fmt, ...) TRACE_CATG(MNML_VERBOSE_SLOT, __fmt, ## __VA_ARGS__)

/*
 * S-expression tracing.
 */

#define TRACE_SEXP(__c)     \
{                           \
  HEADER_SEXP(__c);         \
  lisp_debug(stderr, __c);  \
}

#define TRACE_CATG_SEXP(__k, __c)   \
{                                   \
  if (__k) {                        \
    TRACE_SEXP(__c)                 \
  }                                 \
}

#define TRACE_BIND_SEXP(__c) TRACE_CATG_SEXP(MNML_VERBOSE_BIND, __c)
#define TRACE_CHAN_SEXP(__c) TRACE_CATG_SEXP(MNML_VERBOSE_CHAN, __c)
#define TRACE_CONS_SEXP(__c) TRACE_CATG_SEXP(MNML_VERBOSE_CONS, __c)
#define TRACE_MAKE_SEXP(__c) TRACE_CATG_SEXP(MNML_VERBOSE_MAKE, __c)
#define TRACE_PLUG_SEXP(__c) TRACE_CATG_SEXP(MNML_VERBOSE_PLUG, __c)
#define TRACE_SLAB_SEXP(__c) TRACE_CATG_SEXP(MNML_VERBOSE_SLAB, __c)

#define TRACE_REFC_SEXP(__f, __t, __n, __c) \
{                                           \
  if (MNML_VERBOSE_REFC) {                  \
    HEADER_REFC(__f, __t, __n);             \
    lisp_debug(stderr, __c);                \
  }                                         \
}

#define TRACE_SLOT_SEXP(__i, __c) \
{                                 \
  if (MNML_VERBOSE_SLOT) {        \
    HEADER_SLOT(__i);             \
    lisp_debug(stderr, __c);      \
  }                               \
}

void lisp_debug(FILE * fp, const atom_t atom);

#else

#define TRACE(__fmt, ...)
#define TRACE_BIND(__c)
#define TRACE_CHAN(__c)
#define TRACE_CONS(__c)
#define TRACE_MAKE(__c)
#define TRACE_PLUG(__c)
#define TRACE_REFC(__c)
#define TRACE_SLAB(__c)
#define TRACE_SLOT(__c)

#define TRACE_SEXP(__c)
#define TRACE_BIND_SEXP(__c)
#define TRACE_CHAN_SEXP(__c)
#define TRACE_CONS_SEXP(__c)
#define TRACE_MAKE_SEXP(__c)
#define TRACE_PLUG_SEXP(__c)
#define TRACE_REFC_SEXP(__c)
#define TRACE_SLAB_SEXP(__c)
#define TRACE_SLOT_SEXP(__c)

#endif

// vim: tw=80:sw=2:ts=2:sts=2:et
