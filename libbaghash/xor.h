#ifndef __XOR_H__
#define __XOR_H__

#include <stdint.h>
#include <stdlib.h>

void xor_block (uint8_t *out, const uint8_t *blockA, const uint8_t *blockB,
  size_t block_size);

void xor_blocks (uint8_t *out, const uint8_t *in, size_t block_length, size_t n_blocks);

#endif
