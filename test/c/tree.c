#include "primitives.h"
#include <mnml/debug.h>
#include <mnml/lexer.h>
#include <mnml/tree.h>
#include <mnml/utils.h>

/*
 * Interpreter.
 */

static lexer_t lexer;
static atom_t result = NULL;

static void
lisp_consumer(UNUSED const lisp_t lisp, const atom_t cell)
{
  result = cell;
}

static void
lisp_test_init(const lisp_t lisp)
{
  lisp->slab = slab_new();
  /*
   * Create the lisp->globals and the lexer.
   */
  lisp->globals = lisp_make_nil(lisp);
  lexer = lexer_create(lisp, lisp_consumer);
  /*
   * Setup the debug variables.
   */
#ifdef LISP_ENABLE_DEBUG
  lisp_debug_parse_flags();
#endif
}

static bool
lisp_test_fini(const lisp_t lisp)
{
  X(lisp, lisp->globals);
  lexer_destroy(lexer);
  TRACE("D %ld", lisp->slab->n_alloc - lisp->slab->n_free);
  SLAB_COLLECT(lisp->slab);
  bool v = lisp->slab->n_alloc == lisp->slab->n_free;
  slab_delete(lisp->slab);
  return v;
}

/*
 * Helpers.
 */

static void
bind_left(const lisp_t lisp, const atom_t left, const atom_t parent)
{
  X(lisp, LEFT(parent));
  LEFT(parent) = left;
  PARENT(left) = parent;
}

static void
bind_right(const lisp_t lisp, const atom_t parent, const atom_t right)
{
  X(lisp, RIGHT(parent));
  PARENT(right) = parent;
  RIGHT(parent) = right;
}

static atom_t
make_pair(const lisp_t lisp, const symbol_t symb, const int64_t v)
{
  const atom_t value = lisp_make_number(lisp, v);
  return lisp_cons(lisp, lisp_make_symbol(lisp, symb), value);
}

/*
 * Tests.
 */

bool
test_left_rotation()
{
  struct lisp lisp;
  /*
   * Initialize the interpreter.
   */
  lisp_test_init(&lisp);
  /*
   * NIL root.
   */
  {
    const atom_t root = lisp_make_nil(&lisp);
    const atom_t x = lisp_tree_new(&lisp, root, lisp_make_nil(&lisp));
    const atom_t r = lisp_tree_lrot(root, x);
    ASSERT_TRUE(IS_NULL(r));
    X(&lisp, x, r);
  }
  /*
   * Node without right branch.
   */
  {
    const atom_t root = lisp_make_nil(&lisp);
    const atom_t x = lisp_tree_new(&lisp, root, lisp_make_nil(&lisp));
    const atom_t r = lisp_tree_lrot(x, x);
    ASSERT_TRUE(r == x);
    X(&lisp, r, root);
  }
  /*
   * Node with a right branch without a left branch.
   */
  {
    const atom_t root = lisp_make_nil(&lisp);
    const atom_t x = lisp_tree_new(&lisp, root, lisp_make_nil(&lisp));
    const atom_t y = lisp_tree_new(&lisp, root, lisp_make_nil(&lisp));
    bind_right(&lisp, x, y);
    const atom_t r = lisp_tree_lrot(x, x);
    ASSERT_TRUE(r == y);
    ASSERT_TRUE(PARENT(x) == y);
    ASSERT_TRUE(LEFT(y) == x);
    ASSERT_TRUE(IS_NULL(RIGHT(x)));
    X(&lisp, r, root);
  }
  /*
   * Node with a right branch with a left branch.
   */
  {
    const atom_t root = lisp_make_nil(&lisp);
    const atom_t x = lisp_tree_new(&lisp, root, lisp_make_nil(&lisp));
    const atom_t y = lisp_tree_new(&lisp, root, lisp_make_nil(&lisp));
    const atom_t z = lisp_tree_new(&lisp, root, lisp_make_nil(&lisp));
    bind_right(&lisp, x, y);
    bind_left(&lisp, z, y);
    const atom_t r = lisp_tree_lrot(x, x);
    ASSERT_TRUE(r == y);
    ASSERT_TRUE(PARENT(x) == y);
    ASSERT_TRUE(LEFT(y) == x);
    ASSERT_TRUE(RIGHT(x) == z);
    X(&lisp, r, root);
  }
  /*
   * Clean-up.
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
  /*
   */
  OK;
}

bool
test_right_rotation()
{
  struct lisp lisp;
  /*
   * Initialize the interpreter.
   */
  lisp_test_init(&lisp);
  /*
   * NIL root.
   */
  {
    const atom_t root = lisp_make_nil(&lisp);
    const atom_t x = lisp_tree_new(&lisp, root, lisp_make_nil(&lisp));
    const atom_t r = lisp_tree_rrot(root, x);
    ASSERT_TRUE(r == root);
    X(&lisp, x, r);
  }
  /*
   * Node without left branch.
   */
  {
    const atom_t root = lisp_make_nil(&lisp);
    const atom_t x = lisp_tree_new(&lisp, root, lisp_make_nil(&lisp));
    const atom_t r = lisp_tree_rrot(x, x);
    ASSERT_TRUE(r == x);
    X(&lisp, r, root);
  }
  /*
   * Node with a left branch without a right branch.
   */
  {
    const atom_t root = lisp_make_nil(&lisp);
    const atom_t x = lisp_tree_new(&lisp, root, lisp_make_nil(&lisp));
    const atom_t y = lisp_tree_new(&lisp, root, lisp_make_nil(&lisp));
    bind_left(&lisp, y, x);
    const atom_t r = lisp_tree_rrot(x, x);
    ASSERT_TRUE(r == y);
    ASSERT_TRUE(PARENT(x) == y);
    ASSERT_TRUE(RIGHT(y) == x);
    ASSERT_TRUE(IS_NULL(LEFT(x)));
    X(&lisp, r, root);
  }
  /*
   * Node with a left branch with a right branch.
   */
  {
    const atom_t root = lisp_make_nil(&lisp);
    const atom_t x = lisp_tree_new(&lisp, root, lisp_make_nil(&lisp));
    TRACE_SEXP(x);
    const atom_t y = lisp_tree_new(&lisp, root, lisp_make_nil(&lisp));
    TRACE_SEXP(y);
    const atom_t z = lisp_tree_new(&lisp, root, lisp_make_nil(&lisp));
    TRACE_SEXP(z);
    bind_left(&lisp, y, x);
    bind_right(&lisp, y, z);
    TRACE_SEXP(x);
    const atom_t r = lisp_tree_rrot(x, x);
    ASSERT_TRUE(r == y);
    ASSERT_TRUE(PARENT(x) == y);
    ASSERT_TRUE(RIGHT(y) == x);
    ASSERT_TRUE(LEFT(x) == z);
    X(&lisp, r, root);
  }
  /*
   * Clean-up.
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
  /*
   */
  OK;
}

bool
test_add()
{
  struct lisp lisp;
  /*
   * Initialize the interpreter.
   */
  lisp_test_init(&lisp);
  /*
   * Create the symbols.
   */
  MAKE_SYMBOL_STATIC(sn01, "01", 2);
  const atom_t s01 = make_pair(&lisp, sn01, 1);
  MAKE_SYMBOL_STATIC(sn02, "02", 2);
  const atom_t s02 = make_pair(&lisp, sn02, 2);
  MAKE_SYMBOL_STATIC(sn05, "05", 2);
  const atom_t s05 = make_pair(&lisp, sn05, 5);
  MAKE_SYMBOL_STATIC(sn07, "07", 2);
  const atom_t s07 = make_pair(&lisp, sn07, 7);
  MAKE_SYMBOL_STATIC(sn08, "08", 2);
  const atom_t s08 = make_pair(&lisp, sn08, 8);
  MAKE_SYMBOL_STATIC(sn11, "11", 2);
  const atom_t s11 = make_pair(&lisp, sn11, 11);
  MAKE_SYMBOL_STATIC(sn14, "14", 2);
  const atom_t s14 = make_pair(&lisp, sn14, 14);
  MAKE_SYMBOL_STATIC(sn15, "15", 2);
  const atom_t s15 = make_pair(&lisp, sn15, 15);
  /*
   * Create an empty root.
   */
  atom_t root = lisp_make_nil(&lisp);
  /*
   * Initial insertions and validation.
   */
  root = lisp_tree_add(&lisp, root, s01);
  ASSERT_TRUE(lisp_symbol_match(KEY(root), sn01));
  root = lisp_tree_add(&lisp, root, s02);
  root = lisp_tree_add(&lisp, root, s05);
  root = lisp_tree_add(&lisp, root, s07);
  root = lisp_tree_add(&lisp, root, s08);
  root = lisp_tree_add(&lisp, root, s11);
  root = lisp_tree_add(&lisp, root, s14);
  ASSERT_TRUE(lisp_symbol_match(KEY(root), sn02));
  /*
   * Last insertion and validation.
   */
  root = lisp_tree_add(&lisp, root, s15);
  ASSERT_TRUE(lisp_symbol_match(KEY(root), sn07));
  /*
   * Insert existing key.
   */
  const atom_t s05p = make_pair(&lisp, sn05, 5);
  const atom_t invl = lisp_tree_add(&lisp, root, s05p);
  ASSERT_TRUE(invl == root);
  /*
   * Delete the tree.
   */
  X(&lisp, root);
  /*
   * Clean-up.
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
  /*
   */
  OK;
}

bool
test_get()
{
  struct lisp lisp;
  /*
   * Initialize the interpreter.
   */
  lisp_test_init(&lisp);
  /*
   * Create the symbols.
   */
  MAKE_SYMBOL_STATIC(sn01, "01", 2);
  const atom_t s01 = make_pair(&lisp, sn01, 1);
  MAKE_SYMBOL_STATIC(sn02, "02", 2);
  const atom_t s02 = make_pair(&lisp, sn02, 2);
  MAKE_SYMBOL_STATIC(sn05, "05", 2);
  const atom_t s05 = make_pair(&lisp, sn05, 5);
  MAKE_SYMBOL_STATIC(sn07, "07", 2);
  const atom_t s07 = make_pair(&lisp, sn07, 7);
  MAKE_SYMBOL_STATIC(sn08, "08", 2);
  const atom_t s08 = make_pair(&lisp, sn08, 8);
  MAKE_SYMBOL_STATIC(sn11, "11", 2);
  const atom_t s11 = make_pair(&lisp, sn11, 11);
  MAKE_SYMBOL_STATIC(sn14, "14", 2);
  const atom_t s14 = make_pair(&lisp, sn14, 14);
  MAKE_SYMBOL_STATIC(sn15, "15", 2);
  const atom_t s15 = make_pair(&lisp, sn15, 15);
  /*
   * Create an empty root.
   */
  atom_t root = lisp_make_nil(&lisp);
  /*
   * Initial insertions and validation.
   */
  root = lisp_tree_add(&lisp, root, s01);
  root = lisp_tree_add(&lisp, root, s02);
  root = lisp_tree_add(&lisp, root, s05);
  root = lisp_tree_add(&lisp, root, s07);
  root = lisp_tree_add(&lisp, root, s08);
  root = lisp_tree_add(&lisp, root, s11);
  root = lisp_tree_add(&lisp, root, s14);
  root = lisp_tree_add(&lisp, root, s15);
  /*
   * Get some values.
   */
  const atom_t v0 = lisp_tree_get(&lisp, root, sn11);
  ASSERT_EQUAL(CDR(v0)->number, 11);
  X(&lisp, v0);
  /*
   * Delete the tree.
   */
  X(&lisp, root);
  /*
   * Clean-up.
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
  /*
   */
  OK;
}

bool
test_rem()
{
  struct lisp lisp;
  /*
   * Initialize the interpreter.
   */
  lisp_test_init(&lisp);
  /*
   * Single-item tree.
   */
  {
    /*
     * Create an empty root.
     */
    atom_t root = lisp_make_nil(&lisp);
    /*
     * Insert a single entry.
     */
    MAKE_SYMBOL_STATIC(sn01, "01", 2);
    const atom_t s01 = make_pair(&lisp, sn01, 1);
    root = lisp_tree_add(&lisp, root, s01);
    /*
     * Delete a single entry.
     */
    root = lisp_tree_rem(&lisp, root, sn01);
    /*
     * Check.
     */
    ASSERT_TRUE(IS_NULL(root));
    /*
     * Delete the tree.
     */
    X(&lisp, root);
  }
  /*
   * Two-item tree (NIL, A, B), delete leaf.
   */
  {
    /*
     * Create an empty root.
     */
    atom_t root = lisp_make_nil(&lisp);
    /*
     * Insert the first entry.
     */
    MAKE_SYMBOL_STATIC(sn01, "01", 2);
    const atom_t s01 = make_pair(&lisp, sn01, 1);
    root = lisp_tree_add(&lisp, root, s01);
    /*
     * Insert the second entry.
     */
    MAKE_SYMBOL_STATIC(sn02, "02", 2);
    const atom_t s02 = make_pair(&lisp, sn02, 2);
    root = lisp_tree_add(&lisp, root, s02);
    /*
     * Delete the second entry.
     */
    root = lisp_tree_rem(&lisp, root, sn02);
    /*
     * Check.
     */
    ASSERT_TRUE(lisp_symbol_match(KEY(root), sn01));
    /*
     * Delete the tree.
     */
    X(&lisp, root);
  }
  /*
   * Two-item tree (B, A, NIL), delete leaf.
   */
  {
    /*
     * Create an empty root.
     */
    atom_t root = lisp_make_nil(&lisp);
    /*
     * Insert the first entry.
     */
    MAKE_SYMBOL_STATIC(sn02, "02", 2);
    const atom_t s02 = make_pair(&lisp, sn02, 2);
    root = lisp_tree_add(&lisp, root, s02);
    /*
     * Insert the second entry.
     */
    MAKE_SYMBOL_STATIC(sn01, "01", 2);
    const atom_t s01 = make_pair(&lisp, sn01, 1);
    root = lisp_tree_add(&lisp, root, s01);
    /*
     * Delete the first entry.
     */
    root = lisp_tree_rem(&lisp, root, sn01);
    /*
     * Check.
     */
    ASSERT_TRUE(lisp_symbol_match(KEY(root), sn02));
    /*
     * Delete the tree.
     */
    X(&lisp, root);
  }
  /*
   * Two-item tree (NIL, A, B), delete root.
   */
  {
    /*
     * Create an empty root.
     */
    atom_t root = lisp_make_nil(&lisp);
    /*
     * Insert the first entry.
     */
    MAKE_SYMBOL_STATIC(sn01, "01", 2);
    const atom_t s01 = make_pair(&lisp, sn01, 1);
    root = lisp_tree_add(&lisp, root, s01);
    /*
     * Insert the second entry.
     */
    MAKE_SYMBOL_STATIC(sn02, "02", 2);
    const atom_t s02 = make_pair(&lisp, sn02, 2);
    root = lisp_tree_add(&lisp, root, s02);
    /*
     * Delete the first entry.
     */
    root = lisp_tree_rem(&lisp, root, sn01);
    /*
     * Check.
     */
    ASSERT_TRUE(lisp_symbol_match(KEY(root), sn02));
    /*
     * Delete the tree.
     */
    X(&lisp, root);
  }
  /*
   * Two-item tree (B, A, NIL), delete root.
   */
  {
    /*
     * Create an empty root.
     */
    atom_t root = lisp_make_nil(&lisp);
    /*
     * Insert the first entry.
     */
    MAKE_SYMBOL_STATIC(sn02, "02", 2);
    const atom_t s02 = make_pair(&lisp, sn02, 2);
    root = lisp_tree_add(&lisp, root, s02);
    /*
     * Insert the second entry.
     */
    MAKE_SYMBOL_STATIC(sn01, "01", 2);
    const atom_t s01 = make_pair(&lisp, sn01, 1);
    root = lisp_tree_add(&lisp, root, s01);
    /*
     * Delete the second entry.
     */
    root = lisp_tree_rem(&lisp, root, sn02);
    /*
     * Check.
     */
    ASSERT_TRUE(lisp_symbol_match(KEY(root), sn01));
    /*
     * Delete the tree.
     */
    X(&lisp, root);
  }
  /*
   * Three-item tree (B, A, C), delete root.
   */
  {
    /*
     * Create an empty root.
     */
    atom_t root = lisp_make_nil(&lisp);
    /*
     * Insert the first entry.
     */
    MAKE_SYMBOL_STATIC(sn02, "02", 2);
    const atom_t s02 = make_pair(&lisp, sn02, 2);
    root = lisp_tree_add(&lisp, root, s02);
    /*
     * Insert the second entry.
     */
    MAKE_SYMBOL_STATIC(sn01, "01", 2);
    const atom_t s01 = make_pair(&lisp, sn01, 1);
    root = lisp_tree_add(&lisp, root, s01);
    /*
     * Insert the third entry.
     */
    MAKE_SYMBOL_STATIC(sn03, "03", 2);
    const atom_t s03 = make_pair(&lisp, sn03, 3);
    root = lisp_tree_add(&lisp, root, s03);
    /*
     * Delete an entry.
     */
    root = lisp_tree_rem(&lisp, root, sn02);
    /*
     * Check.
     */
    ASSERT_TRUE(lisp_symbol_match(KEY(root), sn03));
    /*
     * Delete the tree.
     */
    X(&lisp, root);
  }
  /*
   * Four-item tree (B, A, (NIL, C, D)), delete middle.
   */
  {
    /*
     * Create an empty root.
     */
    atom_t root = lisp_make_nil(&lisp);
    /*
     * Insert the first entry.
     */
    MAKE_SYMBOL_STATIC(sn01, "01", 2);
    const atom_t s01 = make_pair(&lisp, sn01, 1);
    root = lisp_tree_add(&lisp, root, s01);
    /*
     * Insert the second entry.
     */
    MAKE_SYMBOL_STATIC(sn02, "02", 2);
    const atom_t s02 = make_pair(&lisp, sn02, 2);
    root = lisp_tree_add(&lisp, root, s02);
    /*
     * Insert the third entry.
     */
    MAKE_SYMBOL_STATIC(sn03, "03", 2);
    const atom_t s03 = make_pair(&lisp, sn03, 3);
    root = lisp_tree_add(&lisp, root, s03);
    /*
     * Insert the fourth entry.
     */
    MAKE_SYMBOL_STATIC(sn04, "04", 2);
    const atom_t s04 = make_pair(&lisp, sn04, 4);
    root = lisp_tree_add(&lisp, root, s04);
    /*
     * Delete the third entry.
     */
    root = lisp_tree_rem(&lisp, root, sn03);
    /*
     * Check.
     */
    ASSERT_TRUE(lisp_symbol_match(KEY(root), sn02));
    /*
     * Delete the tree.
     */
    X(&lisp, root);
  }
  /*
   * Larger tree.
   */
  {
    /*
     * Initialize the interpreter.
     */
    lisp_test_init(&lisp);
    /*
     * Create the symbols.
     */
    MAKE_SYMBOL_STATIC(sn01, "01", 2);
    const atom_t s01 = make_pair(&lisp, sn01, 1);
    MAKE_SYMBOL_STATIC(sn02, "02", 2);
    const atom_t s02 = make_pair(&lisp, sn02, 2);
    MAKE_SYMBOL_STATIC(sn05, "05", 2);
    const atom_t s05 = make_pair(&lisp, sn05, 5);
    MAKE_SYMBOL_STATIC(sn07, "07", 2);
    const atom_t s07 = make_pair(&lisp, sn07, 7);
    MAKE_SYMBOL_STATIC(sn08, "08", 2);
    const atom_t s08 = make_pair(&lisp, sn08, 8);
    MAKE_SYMBOL_STATIC(sn11, "11", 2);
    const atom_t s11 = make_pair(&lisp, sn11, 11);
    MAKE_SYMBOL_STATIC(sn14, "14", 2);
    const atom_t s14 = make_pair(&lisp, sn14, 14);
    MAKE_SYMBOL_STATIC(sn15, "15", 2);
    const atom_t s15 = make_pair(&lisp, sn15, 15);
    /*
     * Create an empty root.
     */
    atom_t root = lisp_make_nil(&lisp);
    /*
     * Initial insertions.
     */
    root = lisp_tree_add(&lisp, root, s01);
    root = lisp_tree_add(&lisp, root, s02);
    root = lisp_tree_add(&lisp, root, s05);
    root = lisp_tree_add(&lisp, root, s07);
    root = lisp_tree_add(&lisp, root, s08);
    root = lisp_tree_add(&lisp, root, s11);
    root = lisp_tree_add(&lisp, root, s14);
    root = lisp_tree_add(&lisp, root, s15);
    /*
     * Erase the root.
     */
    root = lisp_tree_rem(&lisp, root, sn07);
    /*
     * Check.
     */
    ASSERT_TRUE(lisp_symbol_match(KEY(root), sn08));
    /*
     * Delete the tree.
     */
    X(&lisp, root);
  }
  /*
   * Clean-up.
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
  /*
   */
  OK;
}

/*
 * Main.
 */

int
main(UNUSED const int argc, UNUSED char** const argv)
{
  TEST(test_left_rotation);
  TEST(test_right_rotation);
  TEST(test_add);
  TEST(test_get);
  TEST(test_rem);
  return 0;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
