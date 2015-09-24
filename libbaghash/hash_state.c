

#include <stdlib.h>

#include "bitstream.h"
#include "constants.h"
#include "errors.h"
#include "hash_state.h"

int 
hash_state_init (struct hash_state *s, size_t n_blocks, size_t block_size)
{
  s->n_blocks = n_blocks;
  s->block_size = block_size;
  // To avoid an integer overflow, we limit the block size 
  // memory size
  s->buffer = malloc (n_blocks * block_size);

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
  // Simplest design: hash in place with one buffer
  
  // For each block

    // Pick 16 random neighbors

  return ERROR_NONE;
}

int 
hash_state_extract (struct hash_state *s, const void *out, size_t outlen)
{
  return 0;
}


