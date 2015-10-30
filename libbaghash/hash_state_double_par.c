
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matgen/matrix_gen.h"

#include "bitstream.h"
#include "constants.h"
#include "compress.h"
#include "errors.h"
#include "hash_state.h"

struct double_data {
  uint8_t *src;
  uint8_t *dst;
  struct matrix_generator *matgen;
};

int 
hash_statedouble_par_init (struct hash_state *s, UNUSED struct baghash_options *opts)
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
hash_statedouble_par_free (struct hash_state *s)
{
  struct double_data *data = (struct double_data *)s->extra_data;
  if (s->opts->comp_opts.comb == COMB__XOR && !s->opts->n_neighbors) {
    matrix_generator_free (data->matgen);
  }
  free (s->extra_data);
  return ERROR_NONE;
}

int 
hash_statedouble_par_fill (struct hash_state *s, 
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
hash_statedouble_par_mix (struct hash_state *s)
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

#ifdef DEBUG
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
hash_statedouble_par_extract (struct hash_state *s, void *out, size_t outlen)
{
  // For two-buffer design, hash all of the bytes in the src buffer together.
  // TODO: Check for multiplication overflow here.
  return fill_bytes_from_strings (s, out, outlen, 
    s->buffer, s->block_size * s->n_blocks / 2, NULL, 0);
}

