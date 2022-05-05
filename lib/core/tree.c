#include <mnml/tree.h>
#include <mnml/utils.h>

/*
 * Helpers.
 */

static atom_t
tree_minimum(const atom_t node)
{
  /*
   * Handle NIL case.
   */
  if (IS_NULL(node)) {
    return node;
  }
  /*
   * Left-ward descent.
   */
  atom_t x = node;
  while (!IS_NULL(LEFT(x))) {
    x = LEFT(x);
  }
  /*
   * Return.
   */
  return x;
}

static atom_t
tree_successor(const atom_t node)
{
  /*
   * Handle NIL case.
   */
  if (IS_NULL(node)) {
    return node;
  }
  /*
   * If the node has a right child, get its minimum.
   */
  if (!IS_NULL(RIGHT(node))) {
    return tree_minimum(RIGHT(node));
  }
  /*
   * Otherwise, walk up the tree.
   */
  atom_t x = node, y = PARENT(x);
  while (!IS_NULL(y) && x == RIGHT(y)) {
    x = y;
    y = PARENT(x);
  }
  /*
   * Return.
   */
  return y;
}

/*
 * Node allocation.
 */

atom_t
lisp_tree_new(const lisp_t lisp, const atom_t root, const atom_t kv)
{
  /*
   * Atoms.
   */
  const atom_t left = lisp_make_nil(lisp);
  const atom_t parent = IS_NULL(root) ? root : PARENT(root);
  const atom_t right = lisp_make_nil(lisp);
  /*
   * Node.
   */
  const atom_t cons1 = lisp_cons(lisp, parent, right);
  const atom_t cons2 = lisp_cons(lisp, left, cons1);
  const atom_t cons3 = lisp_cons(lisp, kv, cons2);
  /*
   * Mark the CONS with the parent as WEAKREF.
   */
  SET_WEAKREF(cons1);
  /*
   * Return.
   */
  return cons3;
}

/*
 * Rotations.
 */

atom_t
lisp_tree_lrot(const atom_t root, const atom_t x)
{
  atom_t next = root;
  /*
   * Skip of the atom is NIL.
   */
  if (IS_NULL(x)) {
    return next;
  }
  /*
   * Get the right node y.
   */
  const atom_t y = RIGHT(x);
  /*
   * Skip if there is no y.
   */
  if (IS_NULL(y)) {
    return next;
  }
  /*
   * Swap sub-trees.
   */
  RIGHT(x) = LEFT(y);
  if (!IS_NULL(LEFT(y))) {
    PARENT(LEFT(y)) = x;
  }
  /*
   * Link x's parent to y's.
   */
  PARENT(y) = PARENT(x);
  if (IS_NULL(PARENT(x))) {
    next = y;
  } else {
    if (x == LEFT(PARENT(x))) {
      LEFT(PARENT(x)) = y;
    } else {
      RIGHT(PARENT(x)) = y;
    }
  }
  /*
   * Put x on y's left.
   */
  LEFT(y) = x;
  PARENT(x) = y;
  /*
   * Return.
   */
  return next;
}

atom_t
lisp_tree_rrot(const atom_t root, const atom_t x)
{
  atom_t next = root;
  /*
   * Skip of the atom is NIL.
   */
  if (IS_NULL(x)) {
    return next;
  }
  /*
   * Get the left node y.
   */
  const atom_t y = LEFT(x);
  /*
   * Skip if there is no y.
   */
  if (IS_NULL(y)) {
    return next;
  }
  /*
   * Swap sub-trees.
   */
  LEFT(x) = RIGHT(y);
  if (!IS_NULL(RIGHT(y))) {
    PARENT(RIGHT(y)) = x;
  }
  /*
   * Link x's parent to y's.
   */
  PARENT(y) = PARENT(x);
  if (IS_NULL(PARENT(x))) {
    next = y;
  } else {
    if (x == RIGHT(PARENT(x))) {
      RIGHT(PARENT(x)) = y;
    } else {
      LEFT(PARENT(x)) = y;
    }
  }
  /*
   * Put x on y's right.
   */
  RIGHT(y) = x;
  PARENT(x) = y;
  /*
   * Return.
   */
  return next;
}

/*
 * Add operation.
 */

static atom_t
tree_add_node(const lisp_t lisp, const atom_t root, const atom_t node)
{
  /*
   * If the tree is empty, just return the node. Don't deallocate the tree as
   * the parent of the node refers to it.
   */
  if (IS_NULL(root)) {
    return node;
  }
  /*
   * Traverse the tree.
   */
  atom_t y, x = root;
  while (!IS_NULL(x)) {
    /*
     * Update the leaf.
     */
    y = x;
    /*
     * Compare the keys.
     */
    const int cmp = lisp_symbol_compare(KEY(node), &KEY(x)->symbol);
    /*
     * Less.
     */
    if (cmp < 0) {
      x = LEFT(x);
    }
    /*
     * Equal.
     */
    else if (cmp == 0) {
      const atom_t old = VALUE(x);
      VALUE(x) = VALUE(node);
      VALUE(node) = old;
      return lisp_make_true(lisp);
    }
    /*
     * Greater.
     */
    else {
      x = RIGHT(x);
    }
  }
  /*
   * Assign the leaf as the parent of the new node.
   */
  PARENT(node) = y;
  /*
   * Attach the leaf to the new node.
   */
  if (lisp_symbol_compare(KEY(node), &KEY(y)->symbol) < 0) {
    X(lisp, LEFT(y));
    LEFT(y) = node;
  } else {
    X(lisp, RIGHT(y));
    RIGHT(y) = node;
  }
  /*
   * Return.
   */
  return root;
}

static atom_t
tree_add_check(const atom_t root, const atom_t node)
{
  atom_t next = root;
  atom_t x = node;
  /*
   * Traverse the tree and check.
   */
  while (x != next && IS_COLORED(PARENT(x))) {
    if (PARENT(x) == LEFT(PARENT(PARENT(x)))) {
      atom_t y = RIGHT(PARENT(PARENT(x)));
      /*
       * Case 1.
       */
      if (IS_COLORED(y)) {
        CLR_COLOR(PARENT(x));
        CLR_COLOR(y);
        SET_COLOR(PARENT(PARENT(x)));
        x = PARENT(PARENT(x));
      } else {
        /*
         * Case 2.
         */
        if (x == RIGHT(PARENT(x))) {
          x = PARENT(x);
          next = lisp_tree_lrot(next, node);
        }
        /*
         * Case 3.
         */
        CLR_COLOR(PARENT(x));
        SET_COLOR(PARENT(PARENT(x)));
        next = lisp_tree_rrot(next, PARENT(PARENT(x)));
      }
    } else {
      atom_t y = LEFT(PARENT(PARENT(x)));
      /*
       * Case 1.
       */
      if (IS_COLORED(y)) {
        CLR_COLOR(PARENT(x));
        CLR_COLOR(y);
        SET_COLOR(PARENT(PARENT(x)));
        x = PARENT(PARENT(x));
      } else {
        /*
         * Case 2.
         */
        if (x == LEFT(PARENT(x))) {
          x = PARENT(x);
          next = lisp_tree_rrot(next, node);
        }
        /*
         * Case 3.
         */
        CLR_COLOR(PARENT(x));
        SET_COLOR(PARENT(PARENT(x)));
        next = lisp_tree_lrot(next, PARENT(PARENT(x)));
      }
    }
  }
  /*
   * Clear the color of the new root and return it.
   */
  CLR_COLOR(next);
  return next;
}

atom_t
lisp_tree_add(const lisp_t lisp, const atom_t root, const atom_t kv)
{
  TOWEAK(root);
  /*
   * Create the new node.
   */
  const atom_t node = lisp_tree_new(lisp, root, kv);
  SET_COLOR(node);
  /*
   * Insert the new node. Returns NIL if the node exists.
   */
  atom_t next = tree_add_node(lisp, root, node);
  if (IS_TRUE(next)) {
    UNWEAK(root);
    X(lisp, node, next);
    return root;
  }
  /*
   * Check the invariants.
   */
  next = tree_add_check(next, node);
  /*
   * Clear the color of the new root and return it.
   */
  UNWEAK(next);
  return next;
}

/*
 * Update operation.
 */

atom_t
lisp_tree_upd(const lisp_t lisp, const atom_t root, const atom_t kv)
{
  /*
   * Check if the tree is empty.
   */
  if (IS_NULL(root)) {
    return lisp_make_nil(lisp);
  }
  /*
   * Traverse the tree.
   */
  atom_t x = root;
  while (!IS_NULL(x) && !lisp_symbol_match(KEY(x), &CAR(kv)->symbol)) {
    /*
     * Greater or equal.
     */
    if (lisp_symbol_compare(KEY(x), &CAR(kv)->symbol) >= 0) {
      x = LEFT(x);
    }
    /*
     * Less.
     */
    else {
      x = RIGHT(x);
    }
  }
  /*
   * Not found.
   */
  if (IS_NULL(x)) {
    return lisp_make_nil(lisp);
  }
  /*
   * Update the node.
   */
  const atom_t old = VALUE(x);
  VALUE(x) = CDR(kv);
  CDR(kv) = old;
  /*
   * Return.
   */
  return kv;
}

/*
 * Get operation.
 */

static atom_t
tree_get_node(const lisp_t lisp, const atom_t root, const symbol_t key)
{
  /*
   * Check if the tree is empty.
   */
  if (IS_NULL(root)) {
    return lisp_make_nil(lisp);
  }
  /*
   * Traverse the tree.
   */
  atom_t x = root;
  while (!IS_NULL(x) && !lisp_symbol_match(KEY(x), key)) {
    /*
     * Greater or equal.
     */
    if (lisp_symbol_compare(KEY(x), key) >= 0) {
      x = LEFT(x);
    }
    /*
     * Less.
     */
    else {
      x = RIGHT(x);
    }
  }
  /*
   * Not found.
   */
  if (IS_NULL(x)) {
    return lisp_make_nil(lisp);
  }
  /*
   * Return.
   */
  return x;
}

atom_t
lisp_tree_get(const lisp_t lisp, const atom_t root, const symbol_t key)
{
  const atom_t node = tree_get_node(lisp, root, key);
  return IS_NULL(node) ? node : UP(DATA(node));
}

/*
 * Delete operation.
 */

static atom_t
tree_rem_check(const atom_t root, const atom_t parent, const atom_t node)
{
  atom_t next = root;
  atom_t p = parent;
  atom_t x = node;
  /*
   * Otherwise, we need to fix-up the tree.
   */
  while (x != next && !IS_COLORED(x)) {
    /*
     * X is on the left branch.
     */
    if (x == LEFT(p)) {
      atom_t w = RIGHT(p);
      /*
       * Handle case 1.
       */
      if (IS_COLORED(w)) {
        CLR_COLOR(w);
        next = lisp_tree_lrot(next, p);
        w = RIGHT(p);
      }
      /*
       * Handle case 2.
       */
      if (!IS_COLORED(LEFT(w)) && !IS_COLORED(RIGHT(w))) {
        SET_COLOR(w);
        x = p, p = PARENT(x);
      }
      /*
       * Handle case 3 and 4.
       */
      else {
        /*
         * Handle case 3.
         */
        if (!IS_COLORED(RIGHT(w))) {
          CLR_COLOR(LEFT(w));
          SET_COLOR(w);
          next = lisp_tree_rrot(next, w);
          w = RIGHT(p);
        }
        /*
         * Handle case 4.
         */
        IS_COLORED(p) ? SET_COLOR(w) : CLR_COLOR(w);
        CLR_COLOR(p);
        CLR_COLOR(RIGHT(w));
        x = lisp_tree_lrot(next, p);
      }
    }
    /*
     * X is on the right branch.
     */
    else {
      atom_t w = LEFT(p);
      /*
       * Handle case 1.
       */
      if (IS_COLORED(w)) {
        CLR_COLOR(w);
        next = lisp_tree_rrot(next, PARENT(x));
        w = LEFT(p);
      }
      /*
       * Handle case 2.
       */
      if (!IS_COLORED(LEFT(w)) && !IS_COLORED(RIGHT(w))) {
        SET_COLOR(w);
        x = p, p = PARENT(x);
      }
      /*
       * Handle case 3 and 4.
       */
      else {
        /*
         * Handle case 3.
         */
        if (!IS_COLORED(LEFT(w))) {
          CLR_COLOR(RIGHT(w));
          SET_COLOR(w);
          next = lisp_tree_lrot(next, w);
          w = LEFT(p);
        }
        /*
         * Handle case 4.
         */
        IS_COLORED(p) ? SET_COLOR(w) : CLR_COLOR(w);
        CLR_COLOR(p);
        CLR_COLOR(LEFT(w));
        x = lisp_tree_rrot(next, p);
      }
    }
  }
  /*
   * Return the new root.
   */
  return next;
}

atom_t
lisp_tree_rem(const lisp_t lisp, const atom_t root, const symbol_t key)
{
  TOWEAK(root);
  /*
   * Grab the node.
   */
  const atom_t node = tree_get_node(lisp, root, key);
  if (IS_NULL(node)) {
    UNWEAK(root);
    return node;
  }
  /*
   * Remove the node.
   */
  atom_t next = root, p, x, y = node;
  /*
   * Update Y if the node has left and right children.
   */
  if (!IS_NULL(LEFT(node)) && !IS_NULL(RIGHT(node))) {
    y = tree_successor(node);
  }
  /*
   * Position X.
   */
  x = IS_NULL(LEFT(y)) ? RIGHT(y) : LEFT(y);
  p = PARENT(y);
  /*
   * Update X's parent.
   */
  if (!IS_NULL(x)) {
    PARENT(x) = PARENT(y);
  }
  /*
   * If Y has no parent, update the tree's root.
   */
  if (IS_NULL(PARENT(y))) {
    next = x = IS_NULL(x) ? PARENT(y) : UP(x);
  }
  /*
   * Update Y's parent.
   */
  else if (y == LEFT(PARENT(y))) {
    LEFT(PARENT(y)) = UP(x);
  } else {
    RIGHT(PARENT(y)) = UP(x);
  }
  /*
   * If the deleted node is not the original node, save its data.
   */
  if (y != node) {
    const atom_t old = DATA(node);
    DATA(node) = DATA(y);
    DATA(y) = old;
  }
  /*
   * If the removed node is colored, delete it and return.
   */
  if (IS_COLORED(y)) {
    UNWEAK(next);
    X(lisp, y);
    return next;
  }
  /*
   * Otherwise, we need to fix-up the tree.
   */
  next = tree_rem_check(next, p, x);
  UNWEAK(next);
  /*
   * Delete y and return.
   */
  X(lisp, y);
  return next;
}

/*
 * Checks.
 */

atom_t
lisp_tree_dfs(const lisp_t lisp, const atom_t root,
              const lisp_tree_visitor_t cb)
{
  /*
   * Check if the tree is empty.
   */
  if (IS_NULL(root)) {
    return UP(root);
  }
  /*
   * Visit the left side of the tree.
   */
  const atom_t left = lisp_tree_dfs(lisp, LEFT(root), cb);
  if (!IS_NULL(left)) {
    return left;
  }
  X(lisp, left);
  /*
   * Visit the node.
   */
  if (cb(KEY(root), VALUE(root))) {
    return UP(root);
  }
  /*
   * Visit the right side of the tree.
   */
  return lisp_tree_dfs(lisp, RIGHT(root), cb);
}
