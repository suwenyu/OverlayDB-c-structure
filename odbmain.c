#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "odb.h"
typedef unsigned long ulong;
#include "bptree.h"

  static ulong
time_nsec(void)
{
  struct timespec ts;
  // MONO_RAW is 5x to 10x slower than MONO
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000000000lu + ts.tv_nsec;
}

  static void
run(const long nkeys)
{
  printf("%ld keys\n", nkeys);
  struct odb * const tree = odb_create();
  const long range = nkeys * 8;

  // insert
  ulong t0, dt;
  t0 = time_nsec();
  for (long i = 0; i < nkeys; i++) {
    const long k = (i * 71) % range;
    odb_insert(tree, k, i);
  }
  dt = time_nsec() - t0;
  printf("insert time %.3lfs\n", ((double)dt) / 1e9f);
  // print_IO();
  // lookup
  long out;
  long found = 0;
  t0 = time_nsec();
  for (long i = 0; i < nkeys; i++) {
    const long k = (i * 71) % range;
    const bool r = odb_lookup(tree, k, &out);
    if (r) {
      // printf("%4ld %4ld\n", i, out);
      found++;
    }
    
    // break;
  }
  dt = time_nsec() - t0;
  printf("+lookup time %.3lfs found %ld\n", ((double)dt) / 1e9f, found);
  display_table_lookup();
  // print_IO();
  found = 0;
  // lookup for non-existing keys
  t0 = time_nsec();
  for (long i = 0; i < nkeys; i++) {
    const long k = ((i + nkeys) * 71) % range;
    const bool r = odb_lookup(tree, k, &out);
    if (r) {
      // printf("%4ld %4ld\n", i, out);
      found++;
    }
    // break;
  }

  dt = time_nsec() - t0;
  printf("-lookup time %.3lfs found %ld\n", ((double)dt) / 1e9f, found);
  // print_IO();
  display_table_lookup();
  odb_destroy(tree);
}

  int
main(int argc, char ** argv)
{
  if (argc == 1)
    printf("usage %s <nkeys1> ...\n", argv[0]);
  for (int a = 1; a < argc; a++) {
    const long nkeys = strtol(argv[a], NULL, 10);
    run(nkeys);
  }
}
