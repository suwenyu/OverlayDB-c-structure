#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "bptree.h"
#include <stdio.h>

long IO_count=0;
long count_tmp = 0;

struct node;
struct node *approached[100];

struct entry {
  long key;
  union {
    struct node * child; // internal node
    long value; // leaf node
  };
};

#define FANOUT (64)
struct node {
  long nkeys;
  long type; // 0: leaf; 1: internal
  struct entry es[FANOUT];
};

// internal nodes: es[0].key is always ignored
// es[0].child's keys < es[1].key <= es[1].child's keys

struct bptree {
  long depth; // #levels of non-leaf nodes, initially 0
  struct node * root;
};

  struct bptree *
bptree_create(void)
{
  struct bptree * const tree = calloc(1, sizeof(*tree));
  // depth = 0;
  tree->root = calloc(1, sizeof(struct node)); // type == 0; nkeys == 0;
  
  return tree;
}

  static long
node_search(struct node * const leaf, const long key)
{

  int temp=0;
  for(int i=0;i<=count_tmp;i++){
    if(approached[i]==leaf){
      temp=1;
      break;
    }
  }
  if(temp==0){
    IO_count++;
    count_tmp++;
    approached[count_tmp]=leaf;

  }
  long l = 0;
  long r = leaf->nkeys;
  // where to put key
  while (l != r) {
    const long x = (l + r) / 2; // assert(x < leaf->nkeys);
    if (leaf->es[x].key < key) {
      l = x+1;
    } else if (leaf->es[x].key >= key) {
      r = x;
    }
  }
  return l; // key <= leaf->es[l].key
}

  static long
node_search_pivot(struct node * const leaf, const long key)
{
  int temp=0;
  for(int i=0;i<=count_tmp;i++){
    if(approached[i]==leaf){
      temp=1;
      break;
    }
  }
  if(temp==0){
    IO_count++;
    count_tmp++;
    approached[count_tmp]=leaf;

  }

  long l = 0;
  long r = leaf->nkeys - 1;
  // where to put key
  while (l != r) {
    const long x = (l + r + 1) / 2; // assert(x >= 0 && x < leaf->nkeys);
    if (leaf->es[x].key <= key) {
      l = x;
    } else if (leaf->es[x].key > key) {
      r = x-1;
    }
  }
  return l; // key >= leaf->es[l].key
}

  static void
split_path(struct bptree * const tree, struct node ** const path, const long key)
{
  const long depth0 = tree->depth;
  long split_idx = depth0;
  while (split_idx && (path[split_idx-1]->nkeys == FANOUT))
    split_idx--;

  for (long i = split_idx; i <= depth0; i++) { // for each level
    // new left/right
    struct node * const left = path[i];
    struct node * const right = calloc(1, sizeof(struct node));
    right->type = left->type; // never change

    // move items
    memcpy(&(right->es[0]), &(left->es[FANOUT/2]), sizeof(struct entry) * (FANOUT - (FANOUT/2)));
    left->nkeys = FANOUT/2;
    right->nkeys = FANOUT - (FANOUT/2);

    // insert at parent node or lift the root node
    if (i == 0) { // lift root node
      struct node * const newroot = calloc(1, sizeof(struct node));
      newroot->type = 1;
      newroot->nkeys = 2;
      newroot->es[0].key = LONG_MIN; // never used
      newroot->es[0].child = left;
      newroot->es[1].key = right->es[0].key;
      newroot->es[1].child = right;
      tree->root = newroot;
      tree->depth++;
    } else { // insert at parent node
      struct node * const p = path[i-1]; // assert(p->nkeys < FANOUT);
      const long idx = node_search(p, right->es[0].key); // assert(idx > 0);
      memmove(&(p->es[idx+1]), &(p->es[idx]), sizeof(struct entry) * (p->nkeys - idx));
      p->nkeys++;
      p->es[idx].key = right->es[0].key;
      p->es[idx].child = right;
    }
    if (key >= right->es[0].key)
      path[i] = right;
  }
}

  static struct node *
bptree_find_leaf_node(struct node * const root, const long key, struct node ** const path)
{
  long depth = 0;
  struct node * iter = root;
  do {
    // lookup does not need the path
    if (path)
      path[depth] = iter;
    if (iter->type == 0)
      return iter;
    const long idx = node_search_pivot(iter, key);
    iter = iter->es[idx].child;
    depth++;
  } while (true);
}

  bool
bptree_insert(struct bptree * const tree, const long key, const long value)
{
  struct node * path[tree->depth + 1];
  // leaf is also saved at path[tree->depth];
  struct node * const leaf = bptree_find_leaf_node(tree->root, key, path);
  const long idx = node_search(leaf, key);
  if ((idx < leaf->nkeys) && (leaf->es[idx].key == key)){
    memset(approached, 0, sizeof(approached));
    count_tmp=0;
    return false;
  } // already there
    

  // easy case: just insert
  if (leaf->nkeys < FANOUT) {
    // move larger elements right; memmove handles overlaps regions well
    
    memmove(&(leaf->es[idx+1]), &(leaf->es[idx]), sizeof(struct entry) * (leaf->nkeys - idx));
    leaf->nkeys++;
    leaf->es[idx].key = key;
    leaf->es[idx].value = value;
    memset(approached, 0, sizeof(approached));
    count_tmp = 0;
    return true;
  }

  // split and then insert again
  split_path(tree, path, key);
  // retry insert; this will succeed
  return bptree_insert(tree, key, value);
}

  bool
bptree_lookup(struct bptree * const tree, const long key, long * const value_out)
{
  struct node * const leaf = bptree_find_leaf_node(tree->root, key, NULL);
  const long idx = node_search(leaf, key);
  
  memset(approached, 0, sizeof(approached));
  count_tmp = 0;
  
  if ((idx < leaf->nkeys) && (leaf->es[idx].key == key)) {
    *value_out = leaf->es[idx].value;
    
    return true;
  } else {
    return false;
  }
}

  static void
bptree_destroy_rec(struct node * const node)
{
  if (node->type) { // internal
    for (long i = 0; i < node->nkeys; i++)
      bptree_destroy_rec(node->es[i].child);
  }
  free(node);
}

  void
bptree_destroy(struct bptree * const tree)
{
  bptree_destroy_rec(tree->root);
  free(tree);
}

void print_IO(void){
  printf("I/O counts: %ld\n", IO_count);
  IO_count = 0;
}
