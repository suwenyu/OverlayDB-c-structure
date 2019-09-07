#pragma once
#include <stdbool.h>

struct betree;

  extern struct betree *
betree_create(void);

  extern bool
betree_lookup(struct betree * const tree, const long key, long * const value_out);

  extern void
betree_insert(struct betree * const tree, const long key, const long value);

  extern void
betree_destroy(struct betree * const tree);
