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


#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "compress.h"
#include "encode.h"
#include "errors.h"

int 
compress (uint64_t *counter, uint8_t *out, const uint8_t *blocks[], size_t blocks_to_comp)
{
  // TODO: Insert hash metadata (block index and node index) at
  // each compression function call to prevent state reuse.

  SHA256_CTX ctx;
  uint8_t temp[8];

  if (!SHA256_Init (&ctx))
    return ERROR_OPENSSL_HASH;

  uint64_to_littleend_bytes (temp, 8, *counter);
  if (!SHA256_Update (&ctx, temp, 8))
    return ERROR_OPENSSL_HASH;

  for (unsigned int i = 0; i < blocks_to_comp; i++) {
    if (!SHA256_Update (&ctx, blocks[i], BLOCK_SIZE))
      return ERROR_OPENSSL_HASH;
  }

  if (!SHA256_Final (out, &ctx))
    return ERROR_OPENSSL_HASH;

  *counter += 1;

  return ERROR_NONE;
}


int 
expand (uint64_t *counter, uint8_t *buf, size_t blocks_in_buf)
{
  int error;

  const uint8_t *blocks[1] = { buf };
  uint8_t *cur = buf + BLOCK_SIZE;
  for (size_t i = 1; i < blocks_in_buf; i++) { 

    // Block[i] = Hash(Block[i-1])
    if ((error = compress (counter, cur, blocks, 1)))
      return error;

    blocks[0] += BLOCK_SIZE;
    cur += BLOCK_SIZE;
  }

  return ERROR_NONE;
}

