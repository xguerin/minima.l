#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/slab.h>
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
  if (atom->refs == 0xa0a0a0a0a0a0a0aULL) {
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
slab_allocate()
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
slab_destroy(const slab_t slab)
{
  TRACE("D %ld", slab->n_alloc - slab->n_free);
  LISP_COLLECT(slab);
  munmap(slab->entries, SLAB_SIZE);
  free(slab);
}

/*
 * Allocation functions.
 */

atom_t
lisp_allocate(const slab_t slab)
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
   * Mark the slot as used.
   */
  size_t next = slab->first;
  atom_t entry = &slab->entries[next];
  slab->first = entry->next;
  TRACE_SLAB("%ld->%ld 0x%lx", next, slab->first, (uintptr_t)entry);
  /*
   * Prepare the new atom.
   */
#ifdef LISP_ENABLE_DEBUG
  memset(entry, 0, sizeof(struct atom));
#endif
  entry->next = IN_USE;
  slab->n_alloc += 1;
  return entry;
}

static void
lisp_free(const slab_t slab, const atom_t _p)
{
  size_t n = ((uintptr_t)_p - (uintptr_t)slab->entries) / sizeof(struct atom);
  atom_t entry = &slab->entries[n];
  TRACE_SLAB("%ld", n);
#ifdef LISP_ENABLE_DEBUG
  memset(entry, 0xA, sizeof(struct atom));
#endif
  entry->next = slab->first;
  slab->first = n;
  slab->n_free += 1;
}

void
lisp_deallocate(const slab_t slab, const atom_t atom)
{
  TRACE_SLAB_SEXP(atom);
  /*
   * Most likely this is a pair.
   */
  if (likely(IS_PAIR(atom))) {
    X(slab, CAR(atom));
    X(slab, CDR(atom));
    lisp_free(slab, atom);
    return;
  }
  /*
   * Process atoms.
   */
  lisp_free(slab, atom);
}

/*
 * Debug functions.
 */

#ifdef LISP_ENABLE_DEBUG

void
lisp_collect(const slab_t slab)
{
  for (size_t i = 0; i < CELL_COUNT; i += 1) {
    atom_t entry = &slab->entries[i];
    if (entry->next == IN_USE) {
      TRACE_SLOT_SEXP(i, entry);
    }
  }
}

#endif

// vim: tw=80:sw=2:ts=2:sts=2:et
