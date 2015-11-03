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

struct argon2_data {
  size_t round_count;
};

int 
hash_state_argon2_init (struct hash_state *s, UNUSED struct baghash_options *opts)
{
  struct argon2_data *data = malloc (sizeof (*data));
  if (!data)
    return ERROR_MALLOC;

  data->round_count = 0;
  s->extra_data = data;
  return ERROR_NONE; 
}

int
hash_state_argon2_free (struct hash_state *s)
{
  free (s->extra_data);
  return ERROR_NONE;
}

int 
hash_state_argon2_fill (struct hash_state *s, 
    const void *in, size_t inlen,
    const void *salt, size_t saltlen)
{
  // Fill first two blocks of buffer.
  return fill_bytes_from_strings (s, s->buffer, s->block_size, in, inlen, salt, saltlen);
}

int 
hash_state_argon2_mix (struct hash_state *s)
{
  int error;
  uint8_t tmp_block[s->block_size];
  uint64_t neighbor;
  struct argon2_data *data = (struct argon2_data *)s->extra_data;
  
  // Simplest design: hash in place with one buffer
  for (uint64_t i = 0; i < s->n_blocks; i++) {
    const uint8_t *blocks[2];

    // Hash in the previous block (or self if this is the first block
    // in the first pass).
    const size_t prev_idx = (i || data->round_count) ? ((i - 1) % s->n_blocks) : 0;
    const uint8_t *prev_block = block_index (s, prev_idx);
    blocks[0] = prev_block;

  
    // If it's not the first pass through the memory, it's okay to pick any block.
    // If it is the first pass through the memory, pick a block in {1, ..., i-1}. 
    size_t block_max = data->round_count ? s->n_blocks : i;
    //printf("Blockmax: %llu\n", (long long unsigned)block_max);
    if (block_max) {
      if ((error = bitstream_rand_int (&s->bstream, &neighbor, block_max)))
        return error;
    } else {
      neighbor = 0;
    }
    blocks[1] = block_index (s, neighbor);

#if 0
    printf("[%d] prev=%d, other=%d\n", i, (int)prev_idx, (int)neighbor);
#endif
    
    // Hash value of neighbors into temp buffer.
    if ((error = compress (tmp_block, blocks, 2, &s->opts->comp_opts)))
      return error;

    // Copy output of compression function back into the 
    // big memory buffer.
    uint8_t *cur_block = block_index (s, i);
    memcpy (cur_block, tmp_block, s->block_size);
  }

  data->round_count++;

  return ERROR_NONE;
}

int 
hash_state_argon2_extract (struct hash_state *s, void *out, size_t outlen)
{
  // Return bytes derived from the last block of the buffer.
  return fill_bytes_from_strings (s, out, outlen, block_last (s), s->block_size, NULL, 0);
}

