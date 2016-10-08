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

// Both Blake2b and Keccak use this macro
#undef ALIGN
#include "keccak/KeccakSponge.h"

#include "compress.h"
#include "errors.h"
#include "xor.h"


static int 
compress_keccak (uint8_t *out, const uint8_t *blocks[], unsigned int blocks_to_comp)
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
compress_sha512 (uint8_t *out, const uint8_t *blocks[], unsigned int blocks_to_comp)
{
  SHA512_CTX ctx;
  if (!SHA512_Init (&ctx))
    return ERROR_OPENSSL_HASH;
  
  for (unsigned int i = 0; i < blocks_to_comp; i++) {
    if (!SHA512_Update (&ctx, blocks[i], SHA512_DIGEST_LENGTH))
      return ERROR_OPENSSL_HASH;
  }

  if (!SHA512_Final (out, &ctx))
    return ERROR_OPENSSL_HASH;

  return ERROR_NONE;
}

int 
compress (uint8_t *out, const uint8_t *blocks[], 
  size_t blocks_to_comp, enum comp_method comp)
{
  // TODO: Insert hash metadata (block index and node index) at
  // each compression function call to prevent state reuse.
  switch (comp) {
    case COMP__KECCAK_1600:
      return compress_keccak (out, blocks, blocks_to_comp);
    case COMP__SHA_512:
      return compress_sha512 (out, blocks, blocks_to_comp);
    case COMP__ARGON:
    case COMP__END:
      break;
  }

  return ERROR_INVALID_COMPRESSION_METHOD;
}


uint16_t
compress_block_size (enum comp_method comp)
{
  switch (comp)
  {
    case COMP__KECCAK_1600:
      return KECCAK_1600_BLOCK_SIZE;
    case COMP__SHA_512:
      return SHA512_DIGEST_LENGTH;
    case COMP__ARGON:
    case COMP__END:
      break;
  }

  return 0;
}

int 
expand (uint8_t *buf, size_t blocks_in_buf, enum comp_method comp)
{
  int error;
  const uint16_t block_size = compress_block_size (comp);

  const uint8_t *blocks[1] = { buf };
  uint8_t *cur = buf + block_size;
  for (size_t i = 1; i < blocks_in_buf; i++) { 

    // Block[i] = Hash(Block[i-1])
    if ((error = compress (cur, blocks, 1, comp)))
      return error;

    blocks[0] += block_size;
    cur += block_size;
  }

  return ERROR_NONE;
}

