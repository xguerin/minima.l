#pragma once

#include <mnml/types.h>
#include <stdbool.h>
#include <stdlib.h>

/*
 * Slab types.
 */

typedef struct _slab
{
  size_t first;
  size_t n_alloc;
  size_t n_free;
  size_t n_pages;
  atom_t entries;
} * slab_t;

/*
 * Slab macros.
 */

#define SLAB_SIZE (64ULL * 1024ULL * 1024ULL)
#define PAGE_SIZE 4096ULL

#define CELL_COUNT ((slab->n_pages * PAGE_SIZE) / sizeof(struct _atom))

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

slab_t slab_allocate();
void slab_destroy(const slab_t slab);

/*
 * Allocation functions.
 */

atom_t lisp_allocate(const slab_t slab);
void lisp_deallocate(const slab_t slab, const atom_t cell);

/*
 * X macro.
 */

#define LISP_X(__s, __a)            \
  do {                              \
    DOWN(__a);                      \
    if (unlikely(__a->refs == 0)) { \
      lisp_deallocate(__s, __a);    \
    }                               \
  } while (0)

#define X_1(__s, _1) \
  do {               \
    LISP_X(__s, _1); \
  } while (0)

#define X_2(__s, _2, ...)  \
  do {                     \
    LISP_X(__s, _2);       \
    X_1(__s, __VA_ARGS__); \
  } while (0)

#define X_3(__s, _3, ...)  \
  do {                     \
    LISP_X(__s, _3);       \
    X_2(__s, __VA_ARGS__); \
  } while (0)

#define X_4(__s, _4, ...)  \
  do {                     \
    LISP_X(__s, _4);       \
    X_3(__s, __VA_ARGS__); \
  } while (0)

#define X_5(__s, _5, ...)  \
  do {                     \
    LISP_X(__s, _5);       \
    X_4(__s, __VA_ARGS__); \
  } while (0)

#define X_6(__s, _6, ...)  \
  do {                     \
    LISP_X(__s, _6);       \
    X_5(__s, __VA_ARGS__); \
  } while (0)

#define X_7(__s, _7, ...)  \
  do {                     \
    LISP_X(__s, _7);       \
    X_6(__s, __VA_ARGS__); \
  } while (0)

#define X_8(__s, _8, ...)  \
  do {                     \
    LISP_X(__s, _8);       \
    X_7(__s, __VA_ARGS__); \
  } while (0)

#define X_(_1, _2, _3, _4, _5, _6, _7, _8, NAME, ...) NAME
#define X(__s, ...) \
  X_(__VA_ARGS__, X_8, X_7, X_6, X_5, X_4, X_3, X_2, X_1)(__s, __VA_ARGS__)

/*
 * Debug functions.
 */

#ifdef LISP_ENABLE_DEBUG

void lisp_collect(const slab_t slab);
#define LISP_COLLECT(__s) lisp_collect(__s)

#else

#define LISP_COLLECT(__s)

#endif

// vim: tw=80:sw=2:ts=2:sts=2:et
