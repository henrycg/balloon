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


//#include <stdio.h>
#include "mutest.h"

#include "libballoon/constants.h"
#include "libballoon/errors.h"

// For testing static methods
#include "libballoon/bitstream.c"


void 
mu_test_bitstream__bytes_to_int (void) 
{
  unsigned char buf[8] = {0, 0, 0, 0, 0, 0, 0, 0 };
  size_t out;
  
  out = bytes_to_int (buf, 8);
  mu_check (out == 0);

  out = bytes_to_int (buf, 2);
  mu_check (out == 0);

  out = bytes_to_int (buf, 0);
  mu_check (out == 0);

  buf[0] = 5;
  out = bytes_to_int (buf, 1);
  mu_check (out == 5);
  
  out = bytes_to_int (buf, 0);
  mu_check (out == 0);

  out = bytes_to_int (buf, 2);
  mu_check (out == (5 * 256));

  out = bytes_to_int (buf, 3);
  mu_check (out == (5 * 256 * 256));

  buf[2] = 123;
  out = bytes_to_int (buf, 3);
  mu_check (out == (5 * 256 * 256 + 123));
}


void 
mu_test_bitstream__fill_buffer (void) 
{
  struct bitstream b;
  mu_ensure (!bitstream_init (&b));

  const unsigned char seed[] = "abcd";
  mu_ensure (!bitstream_seed_add (&b, seed, sizeof (seed)));
  mu_ensure (!bitstream_seed_add (&b, seed, sizeof (seed) - 1));

  const int blen = 16;
  unsigned char bufA[blen];
  unsigned char bufB[blen];

  // This call should fail, since seed hasn't been
  // finalized yet.
  mu_ensure (bitstream_fill_buffer (&b, bufA, blen));
  mu_ensure (!bitstream_seed_finalize (&b));

  mu_ensure (!bitstream_fill_buffer (&b, bufA, blen));
  mu_ensure (!bitstream_fill_buffer (&b, bufB, blen));

  mu_ensure (bitstream_seed_add (&b, seed, 4));

  bool equal = true;
  for (int i=0; i<blen; i++) {
    if (bufA[i] != bufB[i]) equal = false;
  }

  mu_check (!equal);
  mu_ensure (!bitstream_free (&b));
}

void 
mu_test_bitstream__fill_buffer_seeds (void) 
{
  struct bitstream bA, bB;
  mu_ensure (!bitstream_init (&bA));
  mu_ensure (!bitstream_init (&bB));

  const unsigned char seed[] = "abcdefg";
  mu_ensure (!bitstream_seed_add (&bA, seed, sizeof (seed)));
  mu_ensure (!bitstream_seed_add (&bB, seed, sizeof (seed) - 1));

  mu_ensure (!bitstream_seed_finalize (&bA));
  mu_ensure (!bitstream_seed_finalize (&bB));

  const int blen = 16;
  unsigned char bufA[blen];
  unsigned char bufB[blen];

  mu_ensure (!bitstream_fill_buffer (&bA, bufA, blen));
  mu_ensure (!bitstream_fill_buffer (&bB, bufB, blen));

  bool equal = true;
  for (int i=0; i<blen; i++) {
    if (bufA[i] != bufB[i]) equal = false;
  }

  mu_check (!equal);
  mu_ensure (!bitstream_free (&bA));
  mu_ensure (!bitstream_free (&bB));
}

void 
mu_test_bitstream__rand_int (void) 
{
  struct bitstream b;
  mu_ensure (!bitstream_init (&b));

  const unsigned char seed[] = "abcd";
  mu_ensure (!bitstream_seed_add (&b, seed, sizeof (seed)));
  mu_ensure (!bitstream_seed_finalize (&b));

  uint64_t out;
  for (int i=0; i<1000; i++) {
    mu_ensure (!bitstream_rand_uint64 (&b, &out));
  }
  mu_ensure (b.n_refreshes < 5);

  for (int i=0; i<1000; i++) {
    mu_ensure (!bitstream_rand_uint64 (&b, &out));
  }
  mu_ensure (b.n_refreshes < 50);
  mu_ensure (!bitstream_free (&b));
}

void 
mu_test_bitstream__weird_size (void) 
{
  struct bitstream b;
  mu_ensure (!bitstream_init (&b));

  const unsigned char seed[] = "abcde";
  mu_ensure (!bitstream_seed_add (&b, seed, sizeof (seed)));
  mu_ensure (!bitstream_seed_finalize (&b));

  unsigned char bufSmall[3];
  unsigned char bufOdd[(1<<8) + 7];
  unsigned char bufBigOdd[(1<<12) + 7];


  mu_ensure (!bitstream_fill_buffer (&b, bufSmall, sizeof (bufSmall)));
  mu_ensure (!bitstream_fill_buffer (&b, bufOdd, sizeof (bufOdd)));
  mu_ensure (!bitstream_fill_buffer (&b, bufBigOdd, sizeof (bufBigOdd)));

  //for (int i=0; i < sizeof(bufOdd); i++)
  //  printf("%d\n", bufOdd[i]);

  mu_ensure (!bitstream_free (&b));
}

