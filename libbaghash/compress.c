
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "blake2b/argon2-core.h"
#include "keccak/KeccakSponge.h"

#include "compress.h"
#include "errors.h"


static int 
compress_keccak (unsigned char *out, unsigned const char *blocks[], unsigned int blocks_to_comp)
{
  spongeState sponge;

  if (InitSponge (&sponge, KECCAK_RATE, KECCAK_CAPACITY))
    return ERROR_KECCAK;

  for (unsigned int i = 0; i < blocks_to_comp; i++) {
    if (Absorb (&sponge, blocks[i], 8 * KECCAK_1600_BLOCK_SIZE))
      return ERROR_KECCAK;
  }
  
  if (Squeeze (&sponge, out, 8 * KECCAK_1600_BLOCK_SIZE))
    return ERROR_KECCAK;

  return ERROR_NONE;
}

static int 
compress_blake2b (unsigned char *out, unsigned const char *blocks[], unsigned int blocks_to_comp)
{
  Argon2FillBlock (out, blocks[0], (blocks_to_comp > 1 ? blocks[1] : out));
  for (unsigned int i = 2; i < blocks_to_comp; i++) {
    Argon2FillBlock (out, out, blocks[i]);
  }

  return ERROR_NONE;
}

int 
compress(unsigned char *out, unsigned const char *blocks[], unsigned int blocks_to_comp,
    enum comp_method comp)
{
  // TODO: Insert hash metadata (block index and node index) at
  // each compression function call to prevent state reuse.
  switch (comp) {
    case COMP__KECCAK_1600:
      return compress_keccak (out, blocks, blocks_to_comp);
    case COMP__ARGON_BLAKE2B:
      return compress_blake2b (out, blocks, blocks_to_comp);
    case COMP__END:
      break;
  }

  return ERROR_INVALID_COMPRESSION_METHOD;
}

size_t 
compress_block_size (enum comp_method comp)
{
  switch (comp)
  {
    case COMP__KECCAK_1600:
      return KECCAK_1600_BLOCK_SIZE;
    case COMP__ARGON_BLAKE2B:
      return ARGON2_BLOCK_SIZE;
    case COMP__END:
      break;
  }

  return 0;
}

