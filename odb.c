#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include "odb.h"
#include "bptree.h"


#include "xxhash.h"
typedef uint64_t u64; // use unsigned for hash values
// // example of how to efficiently compute multiple hash value for bloom filters
// // A similar approach is also used by LevelDB:
// //    https://github.com/google/leveldb/blob/master/util/bloom.cc
// const u64 hash0 = XXH64(buffer, length, 0);
// const u64 hash1 = (hash0 >> 17) | (hash0 << 47);
// const u64 hash2 = (hash1 >> 17) | (hash1 << 47);
// // hash3 ... hash6

// TODO: add helper functions when necessary
#define MAXKEYS 100
#define hasharray 500
int count = 0;
int immtable_count = 0;

int immtable_lookup_count = 0;

struct filter{
  u64 array[hasharray];
};

struct odb {
  // TODO: Your code here:
  long depth;
  struct bptree *memTable;
  struct bptree *immTable[10000000];
  struct filter *bloom_filter[10000000];
};

struct filter *create_filter(void){
  struct filter * bloom_filter=calloc(1,sizeof(*bloom_filter));
  return bloom_filter;
}
bool filter_key_match(struct filter *bloom_filter, const long key, unsigned long long seed){
  const u64 hash0 = XXH64((void *)&key, sizeof(key), seed);
  // printf("test2\n");
  const u64 hash1 = (hash0 >> 17) | (hash0 << 47);
  const u64 hash2 = (hash1 >> 17) | (hash1 << 47);
  // const u64 hash3 = (hash2 >> 17) | (hash2 << 47);
  // const u64 hash4 = (hash3 >> 17) | (hash3 << 47);


  
  if(bloom_filter->array[hash0%hasharray] != 1){
    return false;
  }
  if(bloom_filter->array[hash1%hasharray] != 1){
    return false;
  }
  if(bloom_filter->array[hash2%hasharray] != 1){
    return false;
  }
  // if(bloom_filter->array[hash3%64] != 1){
  //   return false;
  // }
  // if(bloom_filter->array[hash4%64] != 1){
  //   return false;
  // }
  return true;
}

void insert_filter_key(struct filter *bloom_filter, const long key, unsigned long long seed ){
  
  const u64 hash0 = XXH64((void *)&key, sizeof(key), seed);
  // printf("test\n");
  const u64 hash1 = (hash0 >> 17) | (hash0 << 47);
  const u64 hash2 = (hash1 >> 17) | (hash1 << 47);
  // const u64 hash3 = (hash2 >> 17) | (hash2 << 47);
  // const u64 hash4 = (hash3 >> 17) | (hash3 << 47);
  // printf("test\n");
  // printf("%llu\n", hash0%64);
  // printf("%llu\n", hash1);
  // printf("%llu\n",hash0%64 );
  bloom_filter->array[hash0%hasharray] = 1;
  // printf("%llu\n",hash1%64 );
  bloom_filter->array[hash1%hasharray] = 1;

  bloom_filter->array[hash2%hasharray] = 1;

  // bloom_filter->array[hash3%64] = 1;

  // bloom_filter->array[hash4%64] = 1;
}


  struct odb *
odb_create(void)
{
  // TODO: Your code here:
  struct odb * const tree=calloc(1,sizeof(*tree));
  return tree;

}

  bool
odb_insert(struct odb * const tree, const long key, const long value)
{
  // TODO: Your code here:
  if(count == MAXKEYS){

    tree->immTable[immtable_count]=tree->memTable;
    

    immtable_count++;
    // tree->memTable=bptree_create();
    count = 0;
  }

  if(count == 0){
    tree->memTable=bptree_create();
    tree->bloom_filter[immtable_count] = create_filter();

  }

  if(count <= MAXKEYS){
    if (bptree_insert(tree->memTable,key,value)){
      // printf("test\n");
      insert_filter_key(tree->bloom_filter[immtable_count], key, 0);
      count++;
      return true;
    }
    else
      return false;
  }
  return false;
}

  bool
odb_lookup(struct odb * const tree, const long key, long * const value_out)
{
  // TODO: Your code here:
  if (bptree_lookup(tree->memTable,key,value_out)){
    return true;
  }
  for (int i=immtable_count-1;i>=0;i--){
    
    // printf("%d\n", (filter_key_match(tree->bloom_filter[i], key, 0)));
    // if(filter_key_match(tree->bloom_filter[i], key, 0) == false){
    //   immtable_lookup_count++;
    // }
    if(!(filter_key_match(tree->bloom_filter[i], key, 0))){
      continue;
    }

    immtable_lookup_count++;
    if(bptree_lookup(tree->immTable[i],key,value_out)){

      return true;
    }
    
  }
  return false;
}

  void
odb_destroy(struct odb * const tree)
{
  // TODO: Your code here:
  bptree_destroy(tree->memTable);
  for (int i=immtable_count-1; i >= 0; i--){
    bptree_destroy(tree->immTable[i]);
  }
}

void display_table_lookup(void){
  // printf("%d\n", immtable_count);

  printf("immtable count: %d\n", immtable_lookup_count);
  immtable_lookup_count = 0;
}
