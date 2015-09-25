
#include "mutest.h"

#include "libbaghash/constants.h"
#include "libbaghash/errors.h"

// For testing static methods
#include "libbaghash/bitstream.c"

void 
mu_test_bitstream__bits_in_int (void) 
{
  mu_check ( bits_in_int (0) == 0 );
  mu_check ( bits_in_int (1) == 1 );
  mu_check ( bits_in_int (2) == 2 );
  mu_check ( bits_in_int (3) == 2 );
  mu_check ( bits_in_int (4) == 3 );
  mu_check ( bits_in_int (1<<7) == 8 );

  size_t big = 1;
  mu_check ( bits_in_int (big<<63) == 64 );
  mu_check ( bits_in_int ((big<<63) - 1 ) == 63 );
}


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
mu_test_bitstream__bytes_required (void) 
{
  size_t max;
 
  max = 0;
  mu_check (bytes_required (max) == 0);

  max = 1;
  mu_check (bytes_required (max) == 0);

  max = 128;
  mu_check (bytes_required (max) == 1);

  max = 255;
  mu_check (bytes_required (max) == 1);

  max = 256;
  mu_check (bytes_required (max) == 2);

  max = 1;
  max <<= 63;
  mu_check (bytes_required (max) == 8);

  max = 1;
  max <<= 61;
  mu_check (bytes_required (max) == 8);
}


void 
mu_test_bitstream__rand_byte (void) 
{
  struct bitstream b;
  mu_ensure (!bitstream_init (&b));

  const unsigned char seed[] = "abcd";
  mu_ensure (!bitstream_seed_add (&b, seed, 3));
  mu_ensure (!bitstream_seed_add (&b, seed, 4));

  unsigned char cA, cB;
  // This call should fail, since seed hasn't been
  // finalized yet.
  mu_ensure (bitstream_rand_byte (&b, &cA));
  mu_ensure (!bitstream_seed_finalize (&b));
  mu_ensure (!bitstream_rand_byte (&b, &cA));

  const int blen = 12;
  bool equal = true;
  for (int i=0; i<blen; i++) {
    mu_ensure (!bitstream_rand_byte (&b, &cA));
    mu_ensure (!bitstream_rand_byte (&b, &cB));
    if (cA != cB) equal = false;
  }
  mu_check (!equal);

  for (int i=0; i<1<<16; i++) {
    mu_ensure (!bitstream_rand_byte (&b, &cA));
  }

}

void 
mu_test_bitstream__fill_buffer (void) 
{
  struct bitstream b;
  mu_ensure (!bitstream_init (&b));

  const unsigned char seed[] = "abcd";
  mu_ensure (!bitstream_seed_add (&b, seed, 3));
  mu_ensure (!bitstream_seed_add (&b, seed, 4));

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
}

void 
mu_test_bitstream__fill_buffer_seeds (void) 
{
  struct bitstream bA, bB;
  mu_ensure (!bitstream_init (&bA));
  mu_ensure (!bitstream_init (&bB));

  const unsigned char seed[] = "abcdefg";
  mu_ensure (!bitstream_seed_add (&bA, seed, 3));
  mu_ensure (!bitstream_seed_add (&bB, seed, 4));

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
}

void 
mu_test_bitstream__rand_int (void) 
{
  struct bitstream b;
  mu_ensure (!bitstream_init (&b));

  const unsigned char seed[] = "abcd";
  mu_ensure (!bitstream_seed_add (&b, seed, 4));
  mu_ensure (!bitstream_seed_finalize (&b));

  size_t out, max;
  max = 3ull;
  for (int i=0; i<1000; i++) {
    mu_ensure (!bitstream_rand_int (&b, &out, max));
    mu_ensure (out < max);
  }
  mu_ensure (b.n_refreshes < 5);

  max = 27ull;
  for (int i=0; i<1000; i++) {
    mu_ensure (!bitstream_rand_int (&b, &out, max));
    mu_ensure (out < max);
  }
  mu_ensure (b.n_refreshes < 50);
}


