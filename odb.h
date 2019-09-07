#pragma once
#include <stdbool.h>

struct odb;

  extern struct odb *
odb_create(void);

// return true if newly inserted
// return false if already in the tree
  extern bool
odb_insert(struct odb * const tree, const long key, const long value);

// return true if found; *value_out has the value
// return false if not found; *value_out remains unchanged
  extern bool
odb_lookup(struct odb * const tree, const long key, long * const value_out);

  extern void
odb_destroy(struct odb * const tree);

void display_table_lookup(void);