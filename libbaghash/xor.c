/*
 * Copyright (c) 2015, Henry Corrigan-Gibbs
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */


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
