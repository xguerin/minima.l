#pragma once

#include <mnml/debug.h>
#include <mnml/lisp.h>

/*
 * Slab types.
 */

typedef struct _slab_t
{
  size_t  first;
  size_t  n_alloc;
  size_t  n_free;
  size_t  n_pages;
  atom_t  entries;
}
slab_t;

extern slab_t slab;

/*
 * Slab macros.
 */

#define SLAB_SIZE (32ULL * 1024ULL * 1024ULL * 1024ULL)
#define PAGE_SIZE 4096ULL

#define CELL_COUNT ((slab.n_pages * PAGE_SIZE) / sizeof(struct _atom))

/*
 * Reference count function.
 */

#ifdef LISP_ENABLE_DEBUG

atom_t lisp_incref(const atom_t, const char * const name);
atom_t lisp_decref(const atom_t, const char * const name);

#define UP(__c)   lisp_incref(__c, #__c)
#define DOWN(__c) lisp_decref(__c, #__c)

#else

#define UP(__c)   ((__c)->refs++, (__c))
#define DOWN(__c) ((__c)->refs--, (__c))

#endif

/*
 * Slab functions.
 */

void lisp_slab_allocate();
void lisp_slab_destroy();

atom_t lisp_allocate();
void lisp_deallocate(const atom_t cell);

#define X(__a) {                  \
  DOWN(__a);                      \
  if (unlikely(__a->refs == 0)) { \
    lisp_free(__a);               \
  }                               \
}

void lisp_free(const atom_t atom);

/*
 * Debug functions.
 */

#ifdef LISP_ENABLE_DEBUG

#ifdef __MACH__

#define HEADER_REFS(__f, __t, __n) \
  FPRINTF(stderr, "[%2llu->%2llu] - %s = ", __f, __t, __n)

#define HEADER_SLOT(__i) \
  FPRINTF(stderr, "[SLOT  ] - @%lu = ", __i)

#else

#define HEADER_REFS(__f, __t, __n) \
  FPRINTF(stderr, "[%2lu->%2lu] - %s = ", __f, __t, __n)

#define HEADER_SLOT(__i) \
  FPRINTF(stderr, "[SLOT  ] - @%lu = ", __i)

#endif

#define TRACE_REFS(__f, __t, __c, __n) {  \
  if (MNML_VERBOSE_REFC) {                \
    HEADER_REFS(__f, __t, __n);           \
    lisp_debug(stderr, __c);              \
  }                                       \
}

#define TRACE_SLOT(__i, __c) {        \
  if (MNML_VERBOSE_SLOT) {            \
    HEADER_SLOT(__i);                 \
    lisp_debug(stderr, __c);          \
  }                                   \
}

#define TRACE_SLAB(__fmt, ...) {      \
  if (MNML_VERBOSE_SLAB) {            \
    TRACE(__fmt, __VA_ARGS__);        \
  }                                   \
}

void lisp_collect();
#define LISP_COLLECT() lisp_collect()

#else

#define TRACE_REFS(__f, __t, __c, __n)
#define TRACE_SLOT(__i, __c)
#define TRACE_SLAB(__fmt, ...)
#define LISP_COLLECT()

#endif
