
#include "mutest.h"

#include "libbaghash/bitstream.h"
#include "matgen/sample_row.h"

void 
mu_test_matgen_sample_row (void) {
  struct bitstream b;
  mu_ensure (!bitstream_init (&b));

  const unsigned char seed[] = "abcd";
  mu_ensure (!bitstream_seed_add (&b, seed, sizeof (seed)));
  mu_ensure (!bitstream_seed_add (&b, seed, sizeof (seed) - 1));

  unsigned char cA;
  // This call should fail, since seed hasn't been
  // finalized yet.
  mu_ensure (bitstream_rand_byte (&b, &cA));
  mu_ensure (!bitstream_seed_finalize (&b));
  mu_ensure (!bitstream_rand_byte (&b, &cA));
  
  size_t out;
  size_t nrows = 100;
  mu_ensure (!nonzero_elms_in_row (&b, &out, nrows, 0));
  mu_ensure (out > 0);

  mu_ensure (!bitstream_free (&b));
}

