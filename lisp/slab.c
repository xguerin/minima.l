#include "lisp.h"
#include "slab.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define IN_USE (-2U)
#define END_MK (-1U)

/*
 * Reference count functions.
 */

#ifdef LISP_ENABLE_DEBUG

atom_t
lisp_incref(const atom_t atom, const char * const name)
{
  TRACE_REFS(atom->refs, atom->refs + 1, atom, name);
  atom->refs += 1;
  return atom;
}

atom_t lisp_decref(const atom_t atom, const char * const name)
{
  TRACE_REFS(atom->refs, atom->refs - 1, atom, name);
  atom->refs -= 1;
  return atom;
}

#endif

/*
 * Slab functions.
 */

slab_t slab = { 0 };

void
lisp_slab_allocate()
{
  memset(&slab, 0, sizeof(slab_t));
  slab.n_pages = 16;
  /*
   * Allocate the slab (32GB).
   */
  slab.entries = (atom_t)mmap(NULL, SLAB_SIZE, PROT_NONE,
                              MAP_ANON | MAP_PRIVATE, -1, 0);
  TRACE("0x%lx", (uintptr_t)slab.entries);
  /*
   * Allocate the first page.
   */
  const size_t size = (slab.n_pages << 1) * PAGE_SIZE;
  int res = mprotect(slab.entries, size, PROT_READ | PROT_WRITE);
  if (res != 0) abort();
  /*
   * Initialize the relative addressing.
   */
  for (size_t i = 0; i < CELL_COUNT; i += 1) {
    slab.entries[i].next = i + 1;
  }
  slab.entries[CELL_COUNT - 1].next = END_MK;
}

void
lisp_slab_expand()
{
  const size_t size = (slab.n_pages << 1) * PAGE_SIZE;
  TRACE("0x%lx", (uintptr_t)slab.entries);
  /*
   * Commit twice the amount of memory.
   */
  int res = mprotect(slab.entries, size, PROT_READ | PROT_WRITE);
  if (res != 0) abort();
  /*
   * Initialize the relative addressing.
   */
  for (size_t i = CELL_COUNT; i < 2 * CELL_COUNT; i += 1) {
    slab.entries[i].next = i + 1;
  }
  slab.entries[2 * CELL_COUNT - 1].next = END_MK;
  /*
   * Update state.
   */
  slab.first = CELL_COUNT;
  slab.n_pages <<= 1;
}

void
lisp_slab_destroy()
{
  munmap(slab.entries, SLAB_SIZE);
}

/*
 * Functions.
 */

atom_t
lisp_allocate() {
  /*
   * Expand if necessary.
   */
  if (unlikely(slab.first == END_MK)) {
    lisp_slab_expand();
  }
  /*
   * Mark the slot as used.
   */
  size_t next = slab.first;
  atom_t entry = &slab.entries[next];
  slab.first = entry->next;
  TRACE("%ld->%ld 0x%lx", next, slab.first, (uintptr_t)entry);
  /*
   * Prepare the new atom.
   */
#ifdef LISP_ENABLE_DEBUG
  memset(entry, 0, sizeof(struct _atom));
#endif
  entry->next = IN_USE;
  slab.n_alloc += 1;
  return entry;
}

void
lisp_deallocate(const atom_t __p) {
  size_t n = ((uintptr_t)__p - (uintptr_t)slab.entries) / sizeof(struct _atom);
  atom_t entry = &slab.entries[n];
  TRACE("%ld", n);
#ifdef LISP_ENABLE_DEBUG
  memset(entry, 0xA, sizeof(struct _atom));
#endif
  entry->next = slab.first;
  slab.first = n;
  slab.n_free += 1;
}

/*
 * Allocation functions.
 */

static void
atom_free_atom(const atom_t atom)
{
  lisp_deallocate(atom);
}

static void
atom_free_string(const atom_t atom)
{
  free((void *)atom->string);
  lisp_deallocate(atom);
}

static void
atom_free_pair(const atom_t atom)
{
  atom_t car = atom->pair.car;
  atom_t cdr = atom->pair.cdr;
  X(cdr); X(car);
  lisp_deallocate(atom);
}

static void (* atom_free_table[ATOM_TYPES])(const atom_t atom) =
{
  [T_NIL     ] = atom_free_atom,
  [T_TRUE    ] = atom_free_atom,
  [T_NUMBER  ] = atom_free_atom,
  [T_PAIR    ] = atom_free_pair,
  [T_STRING  ] = atom_free_string,
  [T_SYMBOL  ] = atom_free_string,
  [T_INLINE  ] = atom_free_atom,
  [T_WILDCARD] = atom_free_atom,
};

void
lisp_free(const atom_t atom)
{
  TRACE_SEXP(atom);
  atom_free_table[atom->type](atom);
}

/*
 * Debug functions.
 */

#ifdef LISP_ENABLE_DEBUG

void
lisp_collect()
{
  for (size_t i = 0; i < CELL_COUNT; i += 1) {
    atom_t entry = &slab.entries[i];
    if (entry->next == IN_USE) {
      TRACE_SLOT(i, entry);
    }
  }
}

#endif
