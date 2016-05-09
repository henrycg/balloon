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

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "bitstream.h"
#include "constants.h"
#include "compress.h"
#include "errors.h"
#include "hash_state_catena.h"

struct catena_data {
  uint8_t *src;
  uint8_t *dst;
  int n_bits;
};

static uint8_t *rel_block_index (struct hash_state *s, uint8_t *buf, size_t idx);

uint64_t
nearest_power_of_two (uint64_t in, int *n_bits)
{
  uint64_t out = 1;
  *n_bits = 0;
  while (out <= in) {
    out <<= 1;
    *n_bits = *n_bits + 1;
  }

  *n_bits = *n_bits - 1;
  return (out >> 1);
}

uint64_t
reverse_bits (uint64_t in, int n_bits)
{
  uint64_t out = 0;

  for (int i = 0; i < n_bits; i++) {
    out <<= 1;
    if (in & 1)
      out |= 1;
    in >>= 1;
  }

  return out;
}

int 
hash_state_catena_init (UNUSED struct hash_state *s, UNUSED struct balloon_options *opts)
{
  struct catena_data *data = malloc (sizeof (*data));
  if (!data)
    return ERROR_MALLOC;

  s->n_blocks = nearest_power_of_two (s->n_blocks, &data->n_bits);  
  // We split the main buffer into two pieces, so we need one less
  // bit to index into them.
  data->n_bits--;
  data->src = block_index (s, 0);
  data->dst = block_index (s, s->n_blocks / 2);

  s->extra_data = data;
  return ERROR_NONE; 
}

int
hash_state_catena_free (UNUSED struct hash_state *s)
{
  free (s->extra_data);
  return ERROR_NONE;
}


int 
hash_state_catena_brg_mix (UNUSED struct hash_state *s)
{
  int error;
  struct catena_data *data = (struct catena_data *)s->extra_data;

  const size_t blocks_per_buf = s->n_blocks / 2; 
  for (size_t i = 0; i < blocks_per_buf; i++) {
    void *cur_block = rel_block_index (s, data->dst, i);

    // Hash in the previous block (or the last block if this is
    // the first block of the buffer).
    const uint8_t *prev_block = i ? cur_block - s->block_size : rel_block_index (s, data->src, blocks_per_buf - 1);

    const uint8_t *blocks[2];
    blocks[0] = prev_block;
    blocks[1] = rel_block_index (s, data->src, reverse_bits (i, data->n_bits));

    // Hash value of neighbors into temp buffer.
    if ((error = compress (cur_block, blocks, 2, &s->opts->comp_opts)))
      return error;
  }

  // Swap the role of the src and dst buffers.
  // At the entry of the mix function, we are always copying from src to dst.
  uint8_t *tmp = data->src;
  data->src = data->dst;
  data->dst = tmp; 

  return ERROR_NONE;
}

static uint8_t *
rel_block_index (struct hash_state *s, uint8_t *buf, size_t idx)
{
  return buf + (s->block_size * idx);
}

