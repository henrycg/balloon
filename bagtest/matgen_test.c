
#include "mutest.h"

#include "libbaghash/bitstream.h"
#include "matgen/matrix_gen.h"

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

  size_t nrows = 100;
  struct matrix_generator *g = matrix_generator_init (&b, nrows, 10);  
  size_t out;
  for (int i=0; i<100; i++) {
    mu_ensure (!matrix_generator_row_weight (g, &out));
    //printf ("Out=%d\n", (int)out);
  }
  mu_ensure (out > 0);

  matrix_generator_free (g);
  mu_ensure (!bitstream_free (&b));
}

