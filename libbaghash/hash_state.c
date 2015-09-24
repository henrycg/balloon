

#include <stdlib.h>

#include "bitstream.h"
#include "constants.h"
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

  return (!s->buffer) ?  ERROR_NONE : ERROR_MALLOC;
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

int 
hash_state_mix (struct hash_state *s)
{
  int error;
  unsigned char tmp_block[s->block_size];
  size_t neighb;
  
  // Simplest design: hash in place with one buffer
  for (size_t i = 0; i < s->n_blocks; i++) {
    memset (tmp_block, 0, s->block_size);

    // For each block, pick random neighbors
    for (size_t n = 0; n < N_NEIGHBORS; n++) { 

      // Get next neighbor
      if ((error = bitstream_rand_int (&s->bstream, &neighb, s->n_blocks)))
        return error;

      // Hash value of next neighbor into temp buffer.

    }

    // Copy output of compression function back into the 
    // big memory buffer.
    memcpy (s->buffer + (s->n_blocks * i), tmp_block, s->block_size);
  }

  return ERROR_NONE;
}

int 
hash_state_extract (struct hash_state *s, const void *out, size_t outlen)
{
  return 0;
}


