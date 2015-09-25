

#include <stdlib.h>
#include <string.h>

#include "bitstream.h"
#include "constants.h"
#include "compress.h"
#include "errors.h"
#include "hash_state.h"

int 
hash_state_init (struct hash_state *s, size_t n_blocks, size_t block_size, 
    const void *salt, size_t saltlen)
{
  s->n_blocks = n_blocks;
  s->block_size = block_size;
  // To avoid an integer overflow, we limit the block size 
  // memory size
  s->buffer = malloc (n_blocks * block_size);

  int error;
  if ((error = bitstream_init_with_seed (&s->bstream, salt, saltlen)))
    return error;

  return (s->buffer) ?  ERROR_NONE : ERROR_MALLOC;
}

void 
hash_state_free (struct hash_state *s)
{
  free (s->buffer);
}

int 
hash_state_fill (struct hash_state *s, 
    const void *in, size_t inlen,
    const void *salt, size_t saltlen)
{
  int error;
  struct bitstream bits;
  if ((error = bitstream_init (&bits)))
    return error;
  if ((error = bitstream_seed_add (&bits, in, inlen)))
    return error;
  if ((error = bitstream_seed_add (&bits, salt, saltlen)))
    return error;
  if ((error = bitstream_seed_finalize (&bits)))
    return error;

  const size_t buflen = s->n_blocks * s->block_size;
  if ((error = bitstream_fill_buffer (&bits, s->buffer, buflen)))
    return error;

  return ERROR_NONE;
}

static inline void *
last_block (const struct hash_state *s)
{
  return s->buffer + (s->block_size * (s->n_blocks - 1));
}

int 
hash_state_mix (struct hash_state *s)
{
  int error;
  unsigned char tmp_block[s->block_size];
  size_t neighbor;
  
  // Simplest design: hash in place with one buffer
  for (size_t i = 0; i < s->n_blocks; i++) {
    void *cur_block = s->buffer + (s->block_size * i);

    const unsigned char *blocks[N_NEIGHBORS + 1];

    // Hash in the previous block (or the last block if this is
    // the first block of the buffer).
    const unsigned char *prev_block = i ? cur_block - s->block_size : last_block (s);
    blocks[0] = prev_block;

    // For each block, pick random neighbors
    for (size_t n = 1; n < N_NEIGHBORS; n++) { 

      // Get next neighbor
      if ((error = bitstream_rand_int (&s->bstream, &neighbor, s->n_blocks)))
        return error;
      blocks[n] = &s->buffer[s->block_size * neighbor];
    }

    // Hash value of neighbors into temp buffer.
    if ((error = compress (tmp_block, blocks, N_NEIGHBORS, s->block_size)))
      return error;

    // Copy output of compression function back into the 
    // big memory buffer.
    memcpy (cur_block, tmp_block, s->block_size);
    //printf("copy %p => %p\n", s->buffer + (s->block_size * i), tmp_block);
  }

  return ERROR_NONE;
}

int 
hash_state_extract (struct hash_state *s, void *out, size_t outlen)
{
  // For one-buffer design, just return the last block of the buffer
  memset (out, 0, outlen);
  memcpy (out, last_block (s), s->block_size);
  return ERROR_NONE;
}


