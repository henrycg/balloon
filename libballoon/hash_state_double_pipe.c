/*
 * Copyright (c) 2015-6, Henry Corrigan-Gibbs
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
#include <stdio.h>
#include <string.h>

#include "simpira/simpira.h"

#include "constants.h"
#include "errors.h"
#include "hash_state_double_pipe.h"
#include "xor.h"

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

struct double_data {
  uint8_t *src;
  uint8_t *dst;
};


static uint8_t *
rel_block_index (struct hash_state *s, uint8_t *buf, size_t idx)
{
  return buf + (s->block_size * idx);
}

int 
hash_state_double_pipe_mix (struct hash_state *s)
{
  int error;
  struct double_data *data = (struct double_data *)s->extra_data;
  if (s->opts->comp_opts.comb != COMB__XOR) 
    return ERROR_DOUBLE_PIPELINE_MUST_USE_XOR;

  const size_t blocks_per_buf = s->n_blocks / 2; 

  // XOR each block with its neighbors
  for (size_t i = 0; i < blocks_per_buf; i++) {
    void *cur_block = rel_block_index (s, data->dst, i);
    size_t n_neighbors = s->opts->n_neighbors;
    uint64_t neighbors[n_neighbors];
    if ((error = bitstream_rand_ints_nodup (&s->bstream, neighbors, &n_neighbors, 
          n_neighbors, blocks_per_buf)))
        return error;

    // For each block, pick random neighbors
    for (size_t n = 0; n < n_neighbors; n++) { 
      // Get next neighbor
      xor_block_self (cur_block,
         rel_block_index (s, data->src, neighbors[n]), s->block_size);
    }
  }

  const size_t pipe_size = 4;  // Number of parallel hashes to compute

  // Hash each block in place
  for (size_t i = 0; i < blocks_per_buf; i++) {
    if (i % pipe_size == 0) { 
      const size_t to_hash = MIN(blocks_per_buf - i, pipe_size);
      //printf("%d %d %d\n", (int)i, (int)to_hash, (int)blocks_per_buf);
      simpira2048_pipe (rel_block_index (s, data->dst, i),
        rel_block_index (s, data->dst, i + 1),
        rel_block_index (s, data->dst, i + 2),
        rel_block_index (s, data->dst, i + 3),
        to_hash);
    }
  }

  // Swap the role of the src and dst buffers.
  // At the entry of the mix function, we are always copying from src to dst.
  uint8_t *tmp = data->src;
  data->src = data->dst;
  data->dst = tmp; 

  return ERROR_NONE;
}

