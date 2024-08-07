#pragma once

#include <mnml/types.h>
#include <stdbool.h>
#include <stdlib.h>

/*
 * Slab types.
 */

typedef struct slab
{
  size_t first;
  size_t n_alloc;
  size_t n_free;
  size_t n_pages;
  atom_t entries;
}* slab_t;

/*
 * Slab macros.
 */

#define SLAB_SIZE (64ULL * 1024ULL * 1024ULL)
#define PAGE_SIZE 4096ULL

#define CELL_COUNT ((slab->n_pages * PAGE_SIZE) / sizeof(struct atom))

/*
 * Reference count function.
 */

#ifdef LISP_ENABLE_DEBUG

atom_t lisp_incref(const atom_t, const char* const name);
atom_t lisp_decref(const atom_t, const char* const name);

#define UP(__c) lisp_incref(__c, #__c)
#define DOWN(__c) lisp_decref(__c, #__c)

#else

#define UP(__c) ((__c)->refs++, (__c))
#define DOWN(__c) ((__c)->refs--, (__c))

#endif

/*
 * Slab functions.
 */

slab_t slab_new();
void slab_delete(const slab_t slab);

/*
 * Allocation functions.
 */

atom_t slab_allocate(const slab_t slab);
void slab_deallocate(const slab_t slab, const atom_t cell);

/*
 * Debug functions.
 */

#ifdef LISP_ENABLE_DEBUG

void slab_collect(const slab_t slab);
#define SLAB_COLLECT(_s) slab_collect(_s)

#else

#define SLAB_COLLECT(_s)

#endif

// vim: tw=80:sw=2:ts=2:sts=2:et
