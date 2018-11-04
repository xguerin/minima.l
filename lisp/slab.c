#include "lisp.h"
#include "slab.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

/*
 * Slab functions.
 */

slab_t slab = { 0 };

void
lisp_slab_allocate()
{
  memset(&slab, 0, sizeof(slab_t));
  slab.n_pages = 1;
  /*
   * Allocate the slab (32GB).
   */
  void * a = mmap(NULL, SLAB_SIZE, PROT_NONE, MAP_ANON | MAP_PRIVATE, -1, 0);
  /*
   * Allocate the first page.
   */
  slab.entries = (entry_t)mmap(a, PAGE_SIZE, PROT_READ | PROT_WRITE,
                               MAP_ANON | MAP_PRIVATE, -1, 0);
  /*
   * Initialize the relative addressing.
   */
  for (size_t i = 0; i < CELL_COUNT; i += 1) {
    const size_t next = i + 1;
    slab.entries[i].next = next == CELL_COUNT ? -1ULL : next;
  }
}

void
lisp_slab_expand()
{
  const size_t size = slab.n_pages * PAGE_SIZE;
  /*
   * Allocate the first page.
   */
  entry_t entries = (entry_t)mmap(slab.entries, size, PROT_READ | PROT_WRITE,
                                  MAP_ANON | MAP_PRIVATE , -1, size);
  if (entries == NULL) abort();
  /*
   * Initialize the relative addressing.
   */
  for (size_t i = 0; i < CELL_COUNT; i += 1) {
    const size_t next = i + 1;
    entries[i].next = next == CELL_COUNT ? -1ULL : CELL_COUNT + next;
  }
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

cell_t
lisp_allocate() {
  if (slab.first == -1ULL) {
    lisp_slab_expand();
  }
  size_t next = slab.first;
  slab.first = slab.entries[next].next;
  cell_t __p = &slab.entries[next].cell;
  memset(__p, 0, sizeof(struct _cell_t));
  slab.n_alloc += 1;
  return __p;
}

void
lisp_deallocate(const cell_t __p) {
  size_t n = ((uintptr_t)__p - (uintptr_t)slab.entries) / sizeof(struct _cell_t);
  slab.entries[n].next = slab.first;
  slab.first = n;
  slab.n_free += 1;
}

/*
 * Allocation functions.
 */

static void
slot_free_noop(const uintptr_t entry)
{

}

static void
slot_free_string(const uintptr_t entry)
{
  free(GET_PNTR(char *, entry));
}

static void
slot_free_list(const uintptr_t entry)
{
  LISP_FREE(GET_PNTR(cell_t, entry));
}

static void (* slot_free_table[8])(const uintptr_t) =
{
  [T_NIL          ] = slot_free_noop,
  [T_LIST         ] = slot_free_list,
  [T_NUMBER       ] = slot_free_noop,
  [T_STRING       ] = slot_free_string,
  [T_SYMBOL       ] = slot_free_string,
  [T_SYMBOL_INLINE] = slot_free_noop,
  [T_TRUE         ] = slot_free_noop,
  [T_WILDCARD     ] = slot_free_noop,
};

void
slot_free(const uintptr_t entry)
{
  slot_free_table[GET_TYPE(entry)](entry);
}

void
lisp_replace(const cell_t cell, const cell_t car)
{
  slot_free(cell->car);
  slot_free(car->cdr);
  cell->car = car->car;
  lisp_deallocate(car);
}

void
lisp_free(const size_t n, ...)
{
  va_list args;
  va_start(args, n);
  /*
   * Deleting the items.
   */
  for (size_t i = 0; i < n; i += 1) {
    cell_t cell = va_arg(args, cell_t);
    if (cell != NIL && cell != TRUE) {
      slot_free(cell->car);
      slot_free(cell->cdr);
      lisp_deallocate(cell);
    }
  }
  /*
   * Clean-up.
   */
  va_end(args);
}
