#pragma once

#include <mnml/debug.h>
#include <mnml/lisp.h>

/*
 * Types.
 */

typedef bool (*lisp_tree_visitor_t)(const atom_t, const atom_t);

/*
 * Accessors.
 */

#define DATA(__x) CAR(__x)
#define KEY(__x) CAR(DATA(__x))
#define VALUE(__x) CDR(DATA(__x))
#define LEFT(__x) CAR(CDR(__x))
#define PARENT(__x) CAR(CDR(CDR(__x)))
#define RIGHT(__x) CDR(CDR(CDR(__x)))

/*
 * WEAKREF handling.
 */

#define TOWEAK(__x)             \
  if (likely(!IS_NULL(__x))) {  \
    SET_WEAKREF(CDR(CDR(__x))); \
  }

#define UNWEAK(__x)             \
  if (likely(!IS_NULL(__x))) {  \
    CLR_WEAKREF(CDR(CDR(__x))); \
  }

/*
 * Node allocation.
 */

atom_t lisp_tree_new(const lisp_t lisp, const atom_t root, const atom_t kv);

/*
 * Rotations.
 */

atom_t lisp_tree_lrot(const atom_t root, const atom_t atom);
atom_t lisp_tree_rrot(const atom_t root, const atom_t atom);

/*
 * Tree operations
 */

atom_t lisp_tree_add(const lisp_t lisp, const atom_t root, const atom_t kv);
atom_t lisp_tree_upd(const lisp_t lisp, const atom_t root, const atom_t kv);
atom_t lisp_tree_get(const lisp_t lisp, const atom_t root, const symbol_t key);
atom_t lisp_tree_rem(const lisp_t lisp, const atom_t root, const symbol_t key);

/*
 * Checks.
 */

atom_t lisp_tree_dfs(const lisp_t lisp, const atom_t root,
                     const lisp_tree_visitor_t cb);
