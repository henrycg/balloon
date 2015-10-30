
#include "xor.h"

void
xor_block (uint8_t *out, const uint8_t *blockA, const uint8_t *blockB,
  size_t block_size)
{
  for (size_t i = 0; i < block_size; i++) {
    out[i] = blockA[i] ^ blockB[i];
  }
}

void 
xor_blocks (uint8_t *out, const uint8_t *in, size_t block_length, size_t n_blocks)
{
  for (size_t i = 0; i < n_blocks; i++) {
    const uint8_t *blockp = in + (block_length * i);
    xor_block (out, out, blockp, block_length);
  }
}
