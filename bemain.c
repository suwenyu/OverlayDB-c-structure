#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "betree.h"
typedef unsigned long ulong;

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
  struct betree * const tree = betree_create();
  const long range = nkeys * 8;

  // insert
  ulong t0, dt;
  t0 = time_nsec();
  for (long i = 0; i < nkeys; i++) {
    const long k = (i * 71) % range;
    betree_insert(tree, k, i);
  }
  dt = time_nsec() - t0;
  printf("insert time %.3lfs\n", ((double)dt) / 1e9f);

  // lookup
  long out;
  long found = 0;
  t0 = time_nsec();
  for (long i = 0; i < nkeys; i++) {
    const long k = (i * 71) % range;
    const bool r = betree_lookup(tree, k, &out);
    if (r) {
      // printf("%4ld %4ld\n", i, out);
      found++;
    }
  }
  dt = time_nsec() - t0;
  printf("+lookup time %.3lfs found %ld\n", ((double)dt) / 1e9f, found);

  found = 0;
  // lookup for non-existing keys
  t0 = time_nsec();
  for (long i = 0; i < nkeys; i++) {
    const long k = ((i + nkeys) * 71) % range;
    const bool r = betree_lookup(tree, k, &out);
    if (r) {
      // printf("%4ld %4ld\n", i, out);
      found++;
    }
  }
  dt = time_nsec() - t0;
  printf("-lookup time %.3lfs found %ld\n", ((double)dt) / 1e9f, found);

  betree_destroy(tree);
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
