
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "keccak/KeccakSponge.h"
#include "compress.h"
#include "errors.h"

int compress (unsigned char *out, unsigned const char *blocks[],
    unsigned int n_blocks, size_t block_size)
{
  // TODO: Maybe use Keccak permutation directly?
  
  assert (8 * block_size == KECCAK_RATE);
  spongeState state;

  if (InitSponge (&state, KECCAK_RATE, KECCAK_CAPACITY))
    return ERROR_KECCAK;

  for (unsigned int i = 0; i < n_blocks; i++) {
    if (Absorb (&state, blocks[i], 8 * block_size))
      return ERROR_KECCAK;
  }
  
  if (Squeeze (&state, out, 8 * block_size))
    return ERROR_KECCAK;

  return ERROR_NONE;
}

