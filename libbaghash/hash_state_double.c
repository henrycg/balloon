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
#include <stdio.h>
#include <string.h>

#include "matgen/matrix_gen.h"

#include "bitstream.h"
#include "constants.h"
#include "compress.h"
#include "errors.h"
#include "hash_state.h"
#include "hash_state_double.h"
#include "xor.h"

struct double_data {
  uint8_t *src;
  uint8_t *dst;
  struct matrix_generator *matgen;
};

int 
hash_state_double_init (struct hash_state *s, UNUSED struct baghash_options *opts)
{
  struct double_data *data = malloc (sizeof (*data));
  if (!data)
    return ERROR_MALLOC;

  data->src = block_index (s, 0);
  data->dst = block_index (s, s->n_blocks / 2);
  if (s->opts->comp_opts.comb == COMB__XOR) {
    if (s->opts->n_neighbors) {
      fprintf(stderr, "Warning: Using non-standard n_neighbors\n"); 
    } else {
      data->matgen = matrix_generator_init (&s->bstream, s->n_blocks / 2);
      if (!data->matgen)
        return ERROR_MALLOC;
    }
  } 
  s->extra_data = data;
  return ERROR_NONE; 
}

int
hash_state_double_free (struct hash_state *s)
{
  struct double_data *data = (struct double_data *)s->extra_data;
  if (s->opts->comp_opts.comb == COMB__XOR && !s->opts->n_neighbors) {
    matrix_generator_free (data->matgen);
  }
  free (s->extra_data);
  return ERROR_NONE;
}

int 
hash_state_double_fill (struct hash_state *s, 
    const void *in, size_t inlen,
    const void *salt, size_t saltlen)
{
  // TODO: Make sure that the multiplication here doesn't overflow
  return fill_bytes_from_strings (s, s->buffer, s->n_blocks * s->block_size / 2, in, inlen, salt, saltlen);
}

static uint8_t *
rel_block_index (struct hash_state *s, uint8_t *buf, size_t idx)
{
  return buf + (s->block_size * idx);
}

int 
hash_state_double_mix (struct hash_state *s)
{
  int error;
  const bool distinct_neighbs = (s->opts->comp_opts.comb == COMB__XOR);
  struct double_data *data = (struct double_data *)s->extra_data;

  const size_t blocks_per_buf = s->n_blocks / 2; 
  for (size_t i = 0; i < blocks_per_buf; i++) {
    void *cur_block = rel_block_index (s, data->dst, i);

    size_t row_neighbors = s->opts->n_neighbors;
    // Only use the fancy distribution if the user has not specified
    // a number of neighbors to use.
    if (s->opts->comp_opts.comb == COMB__XOR && !s->opts->n_neighbors) {
      if ((error = matrix_generator_row_weight (data->matgen, &row_neighbors)))
        return error;
    }
    const size_t n_blocks_to_hash = row_neighbors + 1;
    const uint8_t *blocks[n_blocks_to_hash];

    // Hash in the previous block (or the last block if this is
    // the first block of the buffer).
    const uint8_t *prev_block = i ? cur_block - s->block_size : rel_block_index (s, data->src, blocks_per_buf - 1);
    blocks[0] = prev_block;

    
    // TODO: Check if sorting the neighbors before accessing them improves
    // the performance at all.
    uint64_t neighbors[row_neighbors];
    if ((error = bitstream_rand_ints (&s->bstream, neighbors, 
          row_neighbors, blocks_per_buf, distinct_neighbs)))
      return error;

#if 0
    for (size_t j=0; j<row_neighbors; j++) {
      printf("[%d] (%d) neighb=%d\n", (int)i, (int)j, (int)neighbors[j]);
    }
#endif

    // For each block, pick random neighbors
    for (size_t n = 0; n < row_neighbors; n++) { 
      // Get next neighbor
      blocks[n+1] = rel_block_index (s, data->src, neighbors[n]);
    }

    // Hash value of neighbors into temp buffer.
    if ((error = compress (cur_block, blocks, n_blocks_to_hash, &s->opts->comp_opts)))
      return error;
  }

  // Swap the role of the src and dst buffers.
  // At the entry of the mix function, we are always copying from src to dst.
  uint8_t *tmp = data->src;
  data->src = data->dst;
  data->dst = tmp; 

  return ERROR_NONE;
}

int 
hash_state_double_extract (struct hash_state *s, void *out, size_t outlen)
{
  // For two-buffer design, hash all the last block in the src buffer together.
  // TODO: Check for multiplication overflow here.
  const size_t last_index = (s->n_blocks / 2) - 1;
  const uint8_t *last_block = s->buffer + (last_index * s->block_size);
  const uint8_t *bufp = last_block;
  struct double_data *data = (struct double_data *)s->extra_data;

  if (s->opts->comp_opts.comb == COMB__XOR) {
    // If we are using one of the XOR combining method, XOR the contents
    // of the last buffer together and output that.
    uint8_t tmp[s->block_size];
    memset (tmp, 0, s->block_size);
    for (size_t i = 0; i < s->n_blocks / 2; i++) {
      xor_block (tmp, tmp, rel_block_index (s, data->src, i), s->block_size);
    }
    bufp = tmp;
  } 

  return fill_bytes_from_strings (s, out, outlen, bufp, s->block_size, NULL, 0);
}


