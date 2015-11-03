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


#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "bitstream.h"
#include "constants.h"
#include "compress.h"
#include "errors.h"
#include "hash_state.h"

#define MIN(a, b) ((a < b) ? (a) : (b))

int 
hash_state_single_init (UNUSED struct hash_state *s, UNUSED struct baghash_options *opts)
{
  return ERROR_NONE; 
}

int
hash_state_single_free (UNUSED struct hash_state *s)
{
  return ERROR_NONE;
}

int 
hash_state_single_fill (struct hash_state *s, 
    const void *in, size_t inlen,
    const void *salt, size_t saltlen)
{
  // TODO: Make sure that the multiplication here doesn't overflow
  return fill_bytes_from_strings (s, s->buffer, s->n_blocks * s->block_size, in, inlen, salt, saltlen);
}

int 
hash_state_single_mix (struct hash_state *s)
{
  int error;
  uint8_t tmp_block[s->block_size];
  uint64_t neighbor;
  
  // Simplest design: hash in place with one buffer
  for (size_t i = 0; i < s->n_blocks; i++) {
    void *cur_block = block_index (s, i);

    const size_t n_blocks_to_hash = s->opts->n_neighbors + 2;
    const uint8_t *blocks[n_blocks_to_hash];

    // Hash in the previous block (or the last block if this is
    // the first block of the buffer).
    const uint8_t *prev_block = i ? cur_block - s->block_size : block_last (s);

    blocks[0] = prev_block;
    blocks[1] = cur_block;
    
    // TODO: Check if sorting the neighbors before accessing them improves
    // the performance at all.

    // For each block, pick random neighbors
    for (size_t n = 2; n < n_blocks_to_hash; n++) { 
      // Get next neighbor
      if ((error = bitstream_rand_int (&s->bstream, &neighbor, s->n_blocks)))
        return error;
      blocks[n] = block_index (s, neighbor);
    }

    assert (s->block_size == compress_block_size (s->opts->comp_opts.comp));

    // Hash value of neighbors into temp buffer.
    if ((error = compress (tmp_block, blocks, n_blocks_to_hash, &s->opts->comp_opts)))
      return error;

    // Copy output of compression function back into the 
    // big memory buffer.
    memcpy (cur_block, tmp_block, s->block_size);
    //printf("copy %p => %p\n", s->buffer + (s->block_size * i), tmp_block);
  }

  return ERROR_NONE;
}

int 
hash_state_single_extract (struct hash_state *s, void *out, size_t outlen)
{
  // For one-buffer design, just return bytes derived from
  // the last block of the buffer.
  return fill_bytes_from_strings (s, out, outlen, block_last (s), s->block_size, NULL, 0);
}

