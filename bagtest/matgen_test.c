/*
 * Copyright (c) 2015, Henry Corrigan-Gibbs
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */


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
  struct matrix_generator *g = matrix_generator_init (&b, nrows);  
  size_t out;
  for (int i=0; i<100; i++) {
    mu_ensure (!matrix_generator_row_weight (g, &out));
    //printf ("Out=%d\n", (int)out);
  }
  mu_ensure (out > 0);

  matrix_generator_free (g);
  mu_ensure (!bitstream_free (&b));
}

