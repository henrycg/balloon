/*
 * Copyright (c) 2015-2016, Henry Corrigan-Gibbs
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


#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "bitstream.h"
#include "constants.h"
#include "compress.h"
#include "errors.h"
#include "hash_state.h"

#define MIN(a, b) ((a < b) ? (a) : (b))

void *
block_index (const struct hash_state *s, size_t i)
{
  return s->buffer + (BLOCK_SIZE * i);
}

void *
block_last (const struct hash_state *s)
{
  return block_index (s, s->n_blocks - 1);
}


int 
hash_state_init (struct hash_state *s, struct balloon_options *opts,
    const void *salt, size_t saltlen)
{
  s->n_blocks = options_n_blocks (opts);

  // Force number of blocks to be even
  if (s->n_blocks % 2 != 0) s->n_blocks++;

  s->has_mixed = false;
  s->opts = opts;

  // TODO: Make sure this multiplication doesn't overflow (or use 
  // calloc or realloc)
  s->buffer = malloc (s->n_blocks * BLOCK_SIZE);

  int error;
  if ((error = bitstream_init_with_seed (&s->bstream, salt, saltlen)))
    return error;

  return (s->buffer) ? ERROR_NONE : ERROR_MALLOC;
}

int
hash_state_free (struct hash_state *s)
{
  int error;
  if ((error = bitstream_free (&s->bstream)))
    return error;
  free (s->buffer);
  return ERROR_NONE;
}

int 
hash_state_fill (struct hash_state *s, 
    const void *in, size_t inlen,
    const void *salt, size_t saltlen)
{
  int error;

  // Hash password and salt into 0-th block
  if ((error = fill_bytes_from_strings (s, s->buffer, 
      BLOCK_SIZE, in, inlen, salt, saltlen)))
    return error;

  if ((error = expand (s->buffer, s->n_blocks)))
    return error;

  for (int i=0; i<BLOCK_SIZE;i++) {
    printf("%02x", s->buffer[i]);
  }
  printf("\n");

  return ERROR_NONE;
}

int 
hash_state_mix (struct hash_state *s)
{
  int error;
  uint64_t neighbor;
  
  // Simplest design: hash in place with one buffer
  for (size_t i = 0; i < s->n_blocks; i++) {
    void *cur_block = block_index (s, i);

    const size_t n_blocks_to_hash = 3;
    const uint8_t *blocks[n_blocks_to_hash];

    // Hash in the previous block (or the last block if this is
    // the first block of the buffer).
    const uint8_t *prev_block = i ? cur_block - BLOCK_SIZE : block_last (s);

    blocks[0] = prev_block;
    blocks[1] = cur_block;
    
    // For each block, pick random neighbors
    for (size_t n = 2; n < n_blocks_to_hash; n++) { 
      // Get next neighbor
      if ((error = bitstream_rand_uint64 (&s->bstream, &neighbor)))
        return error;
      //printf("Next[%lu]: %lu\n", i, neighbor % s->n_blocks);
      blocks[n] = block_index (s, neighbor % s->n_blocks);
    }


    // Hash value of neighbors into temp buffer.
    if ((error = compress (cur_block, blocks, n_blocks_to_hash)))
      return error;

    //uint8_t *this_block = cur_block;
    //printf("\t\tBlock[%lu]: %u %u\n", i, this_block[0], this_block[1]);
  }
  s->has_mixed = true;

  s->has_mixed = true;
  return ERROR_NONE;
}

int 
hash_state_extract (struct hash_state *s, void *out, size_t outlen)
{
  if (!s->has_mixed)
    return ERROR_CANNOT_EXTRACT_BEFORE_MIX;

  // Return bytes derived from the last block of the buffer.
  unsigned char *b = block_last (s);
  printf("Last: ");
  for (int i=0; i < BLOCK_SIZE; i++) {
    printf("%x", b[i]);
  }
  puts("");
  return fill_bytes_from_strings (s, out, outlen, block_last (s), BLOCK_SIZE, NULL, 0);
}

int 
fill_bytes_from_strings (__attribute__  ((unused)) struct hash_state *s, 
    uint8_t *block_start, size_t bytes_to_fill,
    const void *in, size_t inlen,
    const void *salt, size_t saltlen)
{
  int error;
  struct bitstream bits;
  if ((error = bitstream_init (&bits)))
    return error;
  if ((error = bitstream_seed_add (&bits, salt, saltlen)))
    return error;
  if ((error = bitstream_seed_add (&bits, in, inlen)))
    return error;
  if ((error = bitstream_seed_finalize (&bits)))
    return error;
  if ((error = bitstream_fill_buffer (&bits, block_start, bytes_to_fill)))
    return error;
  if ((error = bitstream_free (&bits)))
    return error;

  return ERROR_NONE;
}

