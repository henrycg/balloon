
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "bitstream.h"
#include "constants.h"
#include "compress.h"
#include "errors.h"
#include "hash_state.h"

struct double_data {
  unsigned char *src;
  unsigned char *dst;
};

int 
hash_state_double_init (struct hash_state *s, struct baghash_options *opts,
    const void *salt, size_t saltlen)
{
  struct double_data *data = malloc (sizeof (*data));
  if (!data)
    return ERROR_MALLOC;

  data->src = block_index (s, 0);
  data->dst = block_index (s, s->n_blocks / 2);
  s->extra_data = data;
  return ERROR_NONE; 
}

int
hash_state_double_free (struct hash_state *s)
{
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

static unsigned char *
rel_block_index (struct hash_state *s, unsigned char *buf, size_t idx)
{
  return buf + (s->block_size * idx);
}

int 
hash_state_double_mix (struct hash_state *s)
{
  int error;
  size_t neighbor;
  struct double_data *data = (struct double_data *)s->extra_data;

  const size_t blocks_per_buf = s->n_blocks / 2; 
  for (size_t i = 0; i < blocks_per_buf; i++) {
    void *cur_block = rel_block_index (s, data->dst, i);

    const size_t n_blocks_to_hash = s->opts->n_neighbors + 1;
    const unsigned char *blocks[n_blocks_to_hash];

    // Hash in the previous block (or the last block if this is
    // the first block of the buffer).
    const unsigned char *prev_block = i ? cur_block - s->block_size : rel_block_index (s, data->src, blocks_per_buf - 1);

    blocks[0] = prev_block;
    
    // TODO: Check if sorting the neighbors before accessing them improves
    // the performance at all.

    // For each block, pick random neighbors
    for (size_t n = 1; n < n_blocks_to_hash; n++) { 
      // Get next neighbor
      if ((error = bitstream_rand_int (&s->bstream, &neighbor, blocks_per_buf)))
        return error;
      blocks[n] = rel_block_index (s, data->src, neighbor);
    }

    assert (s->block_size == compress_block_size (s->opts->comp_opts.comp));

    // Hash value of neighbors into temp buffer.
    if ((error = compress (cur_block, blocks, n_blocks_to_hash, &s->opts->comp_opts)))
      return error;
  }

  // Swap the role of the src and dst buffers.
  // At the entry of the mix function, we are always copying from src to dst.
  unsigned char *tmp = data->src;
  data->src = data->dst;
  data->dst = tmp; 

  return ERROR_NONE;
}

int 
hash_state_double_extract (struct hash_state *s, void *out, size_t outlen)
{
  // For two-buffer design, hash all of the bytes in the src buffer together.
  // TODO: Check for multiplication overflow here.
  return fill_bytes_from_strings (s, out, outlen, 
    s->buffer, s->block_size * s->n_blocks / 2, NULL, 0);
}

