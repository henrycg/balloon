
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "bitstream.h"
#include "constants.h"
#include "compress.h"
#include "errors.h"
#include "hash_state.h"

#define MIN(a, b) ((a < b) ? (a) : (b))

struct argon2_data {
  size_t round_count;
};

int 
hash_state_argon2_init (struct hash_state *s, struct baghash_options *opts,
    const void *salt, size_t saltlen)
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
  unsigned char tmp_block[s->block_size];
  size_t neighbor;
  struct argon2_data *data = (struct argon2_data *)s->extra_data;
  
  // Simplest design: hash in place with one buffer
  for (int i = 0; i < s->n_blocks; i++) {
    const unsigned char *blocks[2];

    // Hash in the previous block (or self if this is the first block
    // in the first pass).
    const size_t prev_idx = (i || data->round_count) ? ((i - 1) % s->n_blocks) : 0;
    const unsigned char *prev_block = block_index (s, prev_idx);
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
    
    // Hash value of neighbors into temp buffer.
    if ((error = compress (tmp_block, blocks, 2, &s->opts->comp_opts)))
      return error;

    // Copy output of compression function back into the 
    // big memory buffer.
    unsigned char *cur_block = block_index (s, i);
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

