#pragma once

#include "lisp.h"

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
  fprintf(stderr, "! %s:%d: [%llu>%llu] %s = ", __FUNCTION__, __LINE__, __f, __t, __n)

#define HEADER_SLOT(__i) \
  fprintf(stderr, "! %s:%d: @%lu = ", __FUNCTION__, __LINE__, __i)

#else

#define HEADER_REFS(__f, __t, __n) \
  fprintf(stderr, "! %s:%d: [%lu>%lu] %s = ", __FUNCTION__, __LINE__, __f, __t, __n)

#define HEADER_SLOT(__i) \
  fprintf(stderr, "! %s:%d: @%lu = ", __FUNCTION__, __LINE__, __i)

#endif

#define TRACE_REFS(__f, __t, __c, __n) {  \
  HEADER_REFS(__f, __t, __n);             \
  lisp_debug(stderr, __c);                \
}

#define TRACE_SLOT(__i, __c) {  \
  HEADER_SLOT(__i);             \
  lisp_debug(stderr, __c);      \
}

void lisp_collect();
#define LISP_COLLECT() lisp_collect()

#else

#define TRACE_REFS(__f, __t, __c, __n)
#define TRACE_SLOT(__i, __c)
#define LISP_COLLECT()

#endif
