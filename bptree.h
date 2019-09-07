#pragma once
#include <stdbool.h>

struct bptree;

  extern struct bptree *
bptree_create(void);

// return true if newly inserted
// return false if already in the tree
  extern bool
bptree_insert(struct bptree * const tree, const long key, const long value);

// return true if found; *value_out has the value
// return false if not found; *value_out remains unchanged
  extern bool
bptree_lookup(struct bptree * const tree, const long key, long * const value_out);

// deletion is not implemented
  extern void
bptree_destroy(struct bptree * const tree);

extern void print_IO(void);