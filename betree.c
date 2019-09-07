#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include "betree.h"

struct node;

struct entry {
  long key;
  union {
    struct node * child; // interior node
    long value; // for leaf or buffer
  };
};

#define FANOUT   (32) // B
#define EPSILON  (8)  // #pivots
#define BUFFER   (FANOUT - EPSILON)
struct node {
  long type;         // 0: leaf;   >= 1: interior
  struct node * parent; // root's parent is NULL
  long nkeys;   // keys in the leaf or buffer
  long intr_npivots; // interior nodes only
  union {
    struct entry kvs[FANOUT]; // leaf nodes only
    struct { // interior nodes only
      struct entry pivots[EPSILON];
      struct entry buffer[BUFFER];
    };
  };
};

// interior nodes: es[0].key is always ignored
// es[0].child's keys < es[1].key <= es[1].child's keys

struct betree {
  long depth; // #levels of interior nodes, initially 0
  struct node * root;
};

  void
rec_print(struct node * const node, const long depth)
{
static const char * const head = "                                                                 ";
  printf("%.*s", depth * 4, head);
  if (node->type) {
    printf("buffer: ");
    for (long i = 0; i < node->nkeys; i++)
      printf("%ld%c", node->buffer[i].key, i == (node->nkeys -1) ? '\n' : ' ');
    printf("%.*spivots: ", depth*4, head);
    for (long i = 0; i < node->intr_npivots; i++)
      printf("%ld%c", node->pivots[i].key, i == (node->intr_npivots -1) ? '\n' : ' ');
    for (long i = 0; i < node->intr_npivots; i++)
      rec_print(node->pivots[i].child, depth + 1);
  } else {
    printf("leaf: ");
    for (long i = 0; i < node->nkeys; i++)
      printf("%ld%c", node->kvs[i].key, i == (node->nkeys -1) ? '\n' : ' ');
  }
}

  struct betree *
betree_create(void)
{
  struct betree * const tree = calloc(1, sizeof(*tree));
  // depth = 0;
  tree->root = calloc(1, sizeof(struct node)); // everything is 0
  return tree;
}

// search for __exact match__ in es
// return 0 to nkeys - 1 if found
// return -1 if not found
  static long
match_unordered(struct entry * const es, const long nkeys, const long key)
{
  for (long i = 0; i < nkeys; i++)
    if (es[i].key == key)
      return i;
  return -1;
}

// search for l where key <= es[l].key
// this is for match or insertion on the es array
// return 0 to nkeys
  static long
search_ordered(struct entry * const es, const long nkeys, const long key)
{
  long l = 0;
  long r = nkeys;
  // where to put key
  while (l != r) {
    const long x = (l + r) / 2; // assert(x < nkeys);
    if (es[x].key < key) {
      l = x+1;
    } else if (es[x].key >= key) {
      r = x;
    }
  }
  return l; // key <= es[l].key
}

// search for l where key >= es[l].key
// this is used to find a child at the next level
// es[0].key is never accessed as there is no pivot at [0]
// return 0 to nkeys - 1
  static long
search_pivot(struct entry * const es, const long nkeys, const long key)
{
  long l = 0;
  long r = nkeys - 1;
  // where to put key
  while (l != r) {
    const long x = (l + r + 1) / 2; // assert(x >= 0 && x < nkeys);
    if (es[x].key <= key) {
      l = x;
    } else if (es[x].key > key) {
      r = x-1;
    }
  }
  return l; // key >= es[l].key
}

  bool
betree_lookup(struct betree * const tree, const long key, long * const value_out)
{
  // implement betree lookup
  // ONLY use functions defined above
  // return true/false for found/not-found
  // TODO: your code here:
  return false;
}

// insert a key into a leaf node.
// return false if there is no room
  static bool
leaf_insert(struct node * const leaf, const long key, const long value)
{
  // TRY to insert or update a key in the leaf node
  // this function ONLY look at the given node
  // return true/false for for success/failure
  // TODO: your code here:
  return false;
}

// try insert into an interior node's buffer
// return false if there is no room
  static bool
intr_insert(struct node * const intr, const long key, const long value)
{
  // TRY to insert or update a key in a interior node
  // this function ONLY look at the given node
  // return true/false for for success/failure
  // TODO: your code here:
  return false;
}

  static bool
insert_one_kv(struct node * const node, const long key, const long value)
{
  return node->type ? intr_insert(node, key, value) : leaf_insert(node, key, value);
}

// create a right node and move something from left to right
  static struct node *
split_intr_right(struct node * const left)
{
  struct node * const right = calloc(1, sizeof(struct node));
  // safe type/depth, same parent
  right->type = left->type;
  right->parent = left->parent;
  memmove(right->pivots, &(left->pivots[EPSILON/2]), sizeof(struct entry) * (EPSILON/2));
  left->intr_npivots = EPSILON/2;
  right->intr_npivots = EPSILON/2;
  for (long i = 0; i < EPSILON/2; i++)
    right->pivots[i].child->parent = right;

  // move buffered keys
  const long key1 = right->pivots[0].key;
  long idxleft = 0;
  long idxright = 0;
  for (long i = 0; i < left->nkeys; i++) {
    if (left->buffer[i].key < key1) {
      left->buffer[idxleft++] = left->buffer[i];
    } else {
      right->buffer[idxright++] = left->buffer[i];
    }
  }
  left->nkeys = idxleft;
  right->nkeys = idxright;
  // parent will be updated after this function returned
  return right;
}


  static void
split_root_intr(struct betree * const tree)
{
  struct node * const left = tree->root;
  struct node * const newroot = calloc(1, sizeof(struct node));
  left->parent = newroot;
  struct node * const right = split_intr_right(left);
  newroot->type = left->type + 1; // higher
  newroot->intr_npivots = 2;
  newroot->pivots[0].key = left->pivots[0].key;
  newroot->pivots[0].child = left;
  newroot->pivots[1].key = right->pivots[0].key;
  newroot->pivots[1].child = right;
  tree->root = newroot;
}

  static void
insert_pivot(struct node * const parent, const long key, struct node * const child)
{
  assert(parent->type);
  assert(parent->intr_npivots < EPSILON);
  // insert a pivot/child pair at the given (parent) node
  // TODO: Your code here:
}

// recursively split parents
  static void
split_intr(struct betree * const tree, struct node * const intr)
{
  assert(intr->type > 0);
  assert(intr->intr_npivots == EPSILON);

  if (intr->parent == NULL) {
    split_root_intr(tree);
    return;
  }

  // make room at parent node
  if (intr->parent->intr_npivots == EPSILON)
    split_intr(tree, intr->parent);

  struct node * const right = split_intr_right(intr);
  insert_pivot(intr->parent, right->pivots[0].key, right);
}

// split a full leaf node. split full parents recursively
  static void
split_from_leaf(struct betree * const tree, struct node * const left)
{
  assert(left->type == 0);
  assert(left->nkeys == FANOUT);
  // take care of parents
  if (left->parent) { // has parent
    if (left->parent->intr_npivots == EPSILON)
      split_intr(tree, left->parent);
  } else { // leaf is root
    struct node * const newroot = calloc(1, sizeof(struct node));
    newroot->type = left->type + 1;
    newroot->intr_npivots = 1;
    newroot->pivots[0].key = LONG_MIN; // actually unused
    newroot->pivots[0].child = left;
    tree->root = newroot;
    left->parent = newroot;
  }
  struct node * const right = calloc(1, sizeof(struct node));
  right->type = left->type;
  right->parent = left->parent;
  memmove(right->kvs, left->kvs + (FANOUT/2), sizeof(struct entry) * (FANOUT/2));
  left->nkeys = FANOUT/2;
  right->nkeys = FANOUT/2;
  insert_pivot(left->parent, right->kvs[0].key, right);
}

// flush for interior node (recursion)
  static void
flush_buffer(struct betree * const tree, struct node * const node)
{
  assert(node->type != 0);
  assert(node->nkeys == BUFFER);
  while (node->nkeys) { // nkeys may be changed in recursive calls to flush/split
    const long i = node->nkeys - 1;
    const long cidx = search_pivot(node->pivots, node->intr_npivots, node->buffer[i].key);
    struct node * const child = node->pivots[cidx].child;
    if (insert_one_kv(child, node->buffer[i].key, node->buffer[i].value)) {
      node->nkeys--;
    } else if (child->type) { // full buffer
      flush_buffer(tree, child);
    } else { // full leaf
      split_from_leaf(tree, child);
    }
  }
}

// there could be duplicates across the levels
// we don't know at all if it's a update or insertion
  void
betree_insert(struct betree * const tree, const long key, const long value)
{
  while (insert_one_kv(tree->root, key, value) == false) { // root may change
    //printf("////=======BEGIN\n");
    //rec_print(tree->root, 0);
    //printf("----=======END\n");
    if (tree->root->type) {
      flush_buffer(tree, tree->root);
    } else {
      split_from_leaf(tree, tree->root);
    }
  }
}

  static void
betree_destroy_rec(struct node * const node)
{
  // recursively free all betree nodes
  // implement this function so valgrind does not complain of memory leaks at exit
  // TODO: Your code here:
}

  void
betree_destroy(struct betree * const tree)
{
  betree_destroy_rec(tree->root);
  free(tree);
}
