#include "primitives.h"
#include <lisp/slab.h>

void
syntax_error() { }

/*
 * Tests.
 */

bool
alloc_free_test()
{
  /*
   * Allocate the slab allocator.
   */
  lisp_slab_allocate();
  cell_t cells[16];
  /*
   * Allocate 16 cells.
   */
  ASSERT_EQUAL(slab.first, 0);
  for (size_t i = 0; i < 16; i += 1) {
    cells[i] = lisp_allocate();
  }
  /*
   * Free 16 cells.
   */
  ASSERT_EQUAL(slab.first, 16);
  for (size_t i = 0; i < 16; i += 1) {
    lisp_free(1, cells[i]);
    ASSERT_EQUAL(slab.first, i);
  }
  /*
   * Tests.
   */
  ASSERT_EQUAL(slab.first, 15);
  for (size_t i = slab.first; i != 16;) {
    i = slab.entries[i].next;
    ASSERT_TRUE(i >= 0 && i <= 16);
  }
  /*
   * Free the slab allocator.
   */
  lisp_slab_destroy();
  OK;
}

bool
alloc_full_test()
{
  /*
   * Allocate the slab allocator.
   */
  lisp_slab_allocate();
  cell_t cells[CELL_COUNT];
  /*
   * Allocate all cells.
   */
  ASSERT_EQUAL(slab.first, 0);
  for (size_t i = 0; i < CELL_COUNT; i += 1) {
    cells[i] = lisp_allocate();
  }
  /*
   * Free all cells.
   */
  ASSERT_EQUAL(slab.first, -1ULL);
  for (size_t i = 0; i < CELL_COUNT; i += 1) {
    lisp_free(1, cells[i]);
    ASSERT_EQUAL(slab.first, i);
  }
  /*
   * Tests.
   */
  ASSERT_EQUAL(slab.first, CELL_COUNT - 1);
  for (size_t i = slab.first; i != -1ULL;) {
    i = slab.entries[i].next;
    ASSERT_TRUE(i == -1ULL || i >= 0 && i < CELL_COUNT);
  }
  /*
   * Free the slab allocator.
   */
  lisp_slab_destroy();
  OK;
}

bool
alloc_xpnd_test()
{
  /*
   * Allocate the slab allocator.
   */
  lisp_slab_allocate();
  const size_t count = 2 * CELL_COUNT;
  cell_t cells[count];
  /*
   * Allocate all cells.
   */
  ASSERT_EQUAL(slab.first, 0);
  for (size_t i = 0; i < count; i += 1) {
    cells[i] = lisp_allocate();
  }
  /*
   * Free all cells.
   */
  ASSERT_EQUAL(slab.first, -1ULL);
  for (size_t i = 0; i < count; i += 1) {
    lisp_free(1, cells[i]);
    ASSERT_EQUAL(slab.first, i);
  }
  /*
   * Tests.
   */
  ASSERT_EQUAL(slab.first, count - 1);
  for (size_t i = slab.first; i != -1ULL;) {
    i = slab.entries[i].next;
    ASSERT_TRUE(i == -1ULL || i >= 0 && i < CELL_COUNT);
  }
  /*
   * Free the slab allocator.
   */
  lisp_slab_destroy();
  OK;
}

/*
 * Main.
 */

int
main(const int argc, char ** const argv)
{
  TEST(alloc_free_test);
  TEST(alloc_full_test);
  TEST(alloc_xpnd_test);
  return 0;
}
