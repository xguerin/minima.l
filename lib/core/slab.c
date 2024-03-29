#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/slab.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define END_MK (-1U)

/*
 * Reference count functions.
 */

#ifdef LISP_ENABLE_DEBUG

atom_t
lisp_incref(const atom_t atom, const char* const name)
{
  TRACE_REFC_SEXP(atom->refs, atom->refs + 1, name, atom);
  atom->refs += 1;
  return atom;
}

atom_t
lisp_decref(const atom_t atom, const char* const name)
{
  TRACE_REFC_SEXP(atom->refs, atom->refs - 1, name, atom);
  if (atom->refs == 0xA0A0A0A0UL) {
    TRACE("Double-free error: %s", name);
    abort();
  }
  atom->refs -= 1;
  return atom;
}

#endif

/*
 * Slab functions.
 */

slab_t
slab_new()
{
  slab_t slab = (slab_t)malloc(sizeof(struct slab));
  memset(slab, 0, sizeof(struct slab));
  slab->n_pages = 16;
  /*
   * Allocate the slab (64MB).
   */
  slab->entries =
    (atom_t)mmap(NULL, SLAB_SIZE, PROT_NONE, MAP_ANON | MAP_PRIVATE, -1, 0);
  if (slab->entries == MAP_FAILED) {
    ERROR("Cannot reserve %lluB of slab memory", SLAB_SIZE);
    free(slab);
    return NULL;
  }
  TRACE_SLAB("0x%lx", (uintptr_t)slab->entries);
  /*
   * Allocate the first page.
   */
  const size_t size = (slab->n_pages << 1) * PAGE_SIZE;
  int res = mprotect(slab->entries, size, PROT_READ | PROT_WRITE);
  if (res != 0) {
    ERROR("Cannot allocate %luB of slab pages", size);
    free(slab);
    return NULL;
  }
  /*
   * Initialize the relative addressing.
   */
  for (size_t i = 0; i < CELL_COUNT; i += 1) {
    slab->entries[i].next = i + 1;
  }
  slab->entries[CELL_COUNT - 1].next = END_MK;
  return slab;
}

static bool
slab_expand(const slab_t slab)
{
  const size_t size = (slab->n_pages << 1) * PAGE_SIZE;
  TRACE_SLAB("0x%lx", (uintptr_t)slab->entries);
  /*
   * Check that the requested size is within the slab boundaries.
   */
  if (size > SLAB_SIZE) {
    ERROR("No more pages: requested=%lu, allocated=%llu", size, SLAB_SIZE);
    return false;
  }
  /*
   * Commit twice the amount of memory.
   */
  int res = mprotect(slab->entries, size, PROT_READ | PROT_WRITE);
  if (res != 0) {
    ERROR("Cannot expand slab pages to %lu", size);
    return false;
  }
  /*
   * Initialize the relative addressing.
   */
  for (size_t i = CELL_COUNT; i < 2 * CELL_COUNT; i += 1) {
    slab->entries[i].next = i + 1;
  }
  slab->entries[2 * CELL_COUNT - 1].next = END_MK;
  /*
   * Update state.
   */
  slab->first = CELL_COUNT;
  slab->n_pages <<= 1;
  return true;
}

void
slab_delete(const slab_t slab)
{
  TRACE("D %ld", slab->n_alloc - slab->n_free);
  SLAB_COLLECT(slab);
  munmap(slab->entries, SLAB_SIZE);
  free(slab);
}

/*
 * Allocation functions.
 */

atom_t
slab_allocate(const slab_t slab)
{
  /*
   * Expand if necessary. Die if we can't.
   */
  if (unlikely(slab->first == END_MK)) {
    if (!slab_expand(slab)) {
      TRACE("Out-of-memory error");
      abort();
    }
  }
  /*
   * Allocate the entry.
   */
  size_t next = slab->first;
  atom_t entry = &slab->entries[next];
  slab->first = entry->next;
  slab->n_alloc += 1;
  TRACE_SLAB("%ld->%ld 0x%lx", next, slab->first, (uintptr_t)entry);
  /*
   * Erase and return the entry.
   */
  memset(entry, 0, sizeof(struct atom));
  return entry;
}

void
slab_deallocate(const slab_t slab, const atom_t _p)
{
  size_t n = ((uintptr_t)_p - (uintptr_t)slab->entries) / sizeof(struct atom);
  atom_t entry = &slab->entries[n];
  TRACE_SLAB("%ld", n);
#if 0 // LISP_ENABLE_DEBUG
  memset(entry, 0xA, sizeof(struct atom));
#endif
  entry->next = slab->first;
  entry->type = T_NONE;
  slab->first = n;
  slab->n_free += 1;
}

/*
 * Debug functions.
 */

#ifdef LISP_ENABLE_DEBUG

void
slab_collect(const slab_t slab)
{
  for (size_t i = 0; i < CELL_COUNT; i += 1) {
    atom_t entry = &slab->entries[i];
    if (entry->type != T_NONE) {
      TRACE_SLOT_SEXP(i, entry);
    }
  }
}

#endif

// vim: tw=80:sw=2:ts=2:sts=2:et
