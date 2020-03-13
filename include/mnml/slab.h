#pragma once

#include <mnml/types.h>
#include <stdbool.h>

/*
 * Slab types.
 */

typedef struct _slab_t
{
  size_t first;
  size_t n_alloc;
  size_t n_free;
  size_t n_pages;
  atom_t entries;
} slab_t;

extern slab_t slab;

/*
 * Slab macros.
 */

#define SLAB_SIZE (64ULL * 1024ULL * 1024ULL)
#define PAGE_SIZE 4096ULL

#define CELL_COUNT ((slab.n_pages * PAGE_SIZE) / sizeof(struct _atom))

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

bool slab_allocate();
void slab_destroy();

atom_t lisp_allocate();
void lisp_deallocate(const atom_t cell);

#define LISP_X(__a)                 \
  do {                              \
    DOWN(__a);                      \
    if (unlikely(__a->refs == 0)) { \
      lisp_free(__a);               \
    }                               \
  } while (0)

#define X_1(_1) \
  do {          \
    LISP_X(_1); \
  } while (0)

#define X_2(_2, ...)  \
  do {                \
    LISP_X(_2);       \
    X_1(__VA_ARGS__); \
  } while (0)

#define X_3(_3, ...)  \
  do {                \
    LISP_X(_3);       \
    X_2(__VA_ARGS__); \
  } while (0)

#define X_4(_4, ...)  \
  do {                \
    LISP_X(_4);       \
    X_3(__VA_ARGS__); \
  } while (0)

#define X_5(_5, ...)  \
  do {                \
    LISP_X(_5);       \
    X_4(__VA_ARGS__); \
  } while (0)

#define X_6(_6, ...)  \
  do {                \
    LISP_X(_6);       \
    X_5(__VA_ARGS__); \
  } while (0)

#define X_7(_7, ...)  \
  do {                \
    LISP_X(_7);       \
    X_6(__VA_ARGS__); \
  } while (0)

#define X_8(_8, ...)  \
  do {                \
    LISP_X(_8);       \
    X_7(__VA_ARGS__); \
  } while (0)

#define X_(_1, _2, _3, _4, _5, _6, _7, _8, NAME, ...) NAME
#define X(...) \
  X_(__VA_ARGS__, X_8, X_7, X_6, X_5, X_4, X_3, X_2, X_1)(__VA_ARGS__)

void lisp_free(const atom_t atom);

/*
 * Debug functions.
 */

#ifdef LISP_ENABLE_DEBUG

void lisp_collect();
#define LISP_COLLECT() lisp_collect()

#else

#define LISP_COLLECT()

#endif

// vim: tw=80:sw=2:ts=2:sts=2:et
