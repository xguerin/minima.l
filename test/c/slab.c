#include "primitives.h"
#include <mnml/slab.h>
#include <mnml/utils.h>

void
syntax_error()
{}

/*
 * Tests.
 */

bool
alloc_free_test()
{
  /*
   * Allocate the slab allocator.
   */
  slab_allocate();
  atom_t cells[16];
  /*
   * Allocate 16 cells.
   */
  ASSERT_EQUAL(slab.first, 0);
  for (size_t i = 0; i < 16; i += 1) {
    cells[i] = lisp_allocate();
    cells[i]->refs = 1;
  }
  /*
   * Free 16 cells.
   */
  ASSERT_EQUAL(slab.first, 16);
  for (size_t i = 0; i < 16; i += 1) {
    X(cells[i]);
    ASSERT_EQUAL(slab.first, i);
  }
  /*
   * Tests.
   */
  ASSERT_EQUAL(slab.first, 15);
  for (size_t i = slab.first; i != 16;) {
    i = slab.entries[i].next;
    ASSERT_TRUE(i <= 16);
  }
  /*
   * Free the slab allocator.
   */
  slab_destroy();
  OK;
}

bool
alloc_full_test()
{
  /*
   * Allocate the slab allocator.
   */
  slab_allocate();
  atom_t cells[CELL_COUNT];
  /*
   * Allocate all cells.
   */
  ASSERT_EQUAL(slab.first, 0);
  for (size_t i = 0; i < CELL_COUNT; i += 1) {
    cells[i] = lisp_allocate();
    cells[i]->refs = 1;
  }
  /*
   * Free all cells.
   */
  ASSERT_EQUAL(slab.first, -1U);
  for (size_t i = 0; i < CELL_COUNT; i += 1) {
    X(cells[i]);
    ASSERT_EQUAL(slab.first, i);
  }
  /*
   * Tests.
   */
  ASSERT_EQUAL(slab.first, CELL_COUNT - 1);
  for (size_t i = slab.first; i != -1U;) {
    i = slab.entries[i].next;
    ASSERT_TRUE(i == -1U || i < CELL_COUNT);
  }
  /*
   * Free the slab allocator.
   */
  slab_destroy();
  OK;
}

bool
alloc_xpnd_test()
{
  /*
   * Allocate the slab allocator.
   */
  slab_allocate();
  const size_t count = 2 * CELL_COUNT;
  atom_t cells[count];
  /*
   * Allocate all cells.
   */
  ASSERT_EQUAL(slab.first, 0);
  for (size_t i = 0; i < count; i += 1) {
    cells[i] = lisp_allocate();
    cells[i]->refs = 1;
  }
  /*
   * Free all cells.
   */
  ASSERT_EQUAL(slab.first, -1U);
  for (size_t i = 0; i < count; i += 1) {
    X(cells[i]);
    ASSERT_EQUAL(slab.first, i);
  }
  /*
   * Tests.
   */
  ASSERT_EQUAL(slab.first, count - 1);
  for (size_t i = slab.first; i != -1U;) {
    i = slab.entries[i].next;
    ASSERT_TRUE(i == -1U || i < CELL_COUNT);
  }
  /*
   * Free the slab allocator.
   */
  slab_destroy();
  OK;
}

/*
 * Main.
 */

int
main(UNUSED const int argc, UNUSED char** const argv)
{
  TEST(alloc_free_test);
  TEST(alloc_full_test);
  TEST(alloc_xpnd_test);
  return 0;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
