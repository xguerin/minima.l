#pragma once

#include "lisp.h"

/*
 * Slab types.
 */

typedef union _entry_t
{
  size_t          next;
  struct _cell_t  cell;
}
* entry_t;

typedef struct _slab_t
{
  size_t  first;
  size_t  n_alloc;
  size_t  n_free;
  size_t  n_pages;
  entry_t entries;
}
slab_t;

extern slab_t slab;

/*
 * Slab macros.
 */

#define SLAB_SIZE (32ULL * 1024ULL * 1024ULL * 1024ULL)
#define PAGE_SIZE 4096ULL

#define CELL_COUNT ((slab.n_pages * PAGE_SIZE) / sizeof(struct _cell_t))

/*
 * Slab functions.
 */

void lisp_slab_allocate();
void lisp_slab_destroy();

cell_t lisp_allocate();
void lisp_deallocate(const cell_t cell);

void lisp_replace(const cell_t cell, const cell_t car);

void slot_free(const uintptr_t slot);
void lisp_free(const size_t n, ...);

/*
 * Helpers.
 */

#define PP_NARG(...)  PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define PP_RSEQ_N() 8,7,6,5,4,3,2,1,0

#define LISP_FREE(...)  \
  lisp_free(PP_NARG(__VA_ARGS__), __VA_ARGS__)
