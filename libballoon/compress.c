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

#include "blake2b/argon2-core.h"
#include "blake2b/blake2.h"
// Both Blake2b and Keccak use this macro
#undef ALIGN
#include "echo/echo.h"
#include "keccak/KeccakSponge.h"
#include "sempira/sempira.h"

#include "compress.h"
#include "errors.h"
#include "xor.h"

static int compress_hash (uint8_t *out, const uint8_t *blocks[], size_t blocks_to_comp,
    enum comp_method meth);
static int compress_xor (uint8_t *out, const uint8_t *blocks[], size_t blocks_to_comp,
    enum comp_method meth);

int 
compress (uint8_t *out, const uint8_t *blocks[], 
  size_t blocks_to_comp, struct comp_options *opts)
{
  switch (opts->comb) {
    case COMB__HASH:
      return compress_hash (out, blocks, blocks_to_comp, opts->comp);
    case COMB__XOR:
      return compress_xor (out, blocks, blocks_to_comp, opts->comp);
    default:
      return ERROR_INVALID_COMPRESSION_METHOD;
  }
}


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
compress_argon (uint8_t *out, const uint8_t *blocks[], unsigned int blocks_to_comp)
{
  if (blocks_to_comp > 1) {
    Argon2FillBlock (out, blocks[0], blocks[1]);
    for (unsigned int i = 2; i < blocks_to_comp; i++) {
      Argon2FillBlock (out, out, blocks[i]);
    }
  } else {
    uint8_t zero[ARGON2_BLOCK_SIZE];
    memset (zero, 0, ARGON2_BLOCK_SIZE);
    Argon2FillBlock (out, blocks[0], zero);
  }

  return ERROR_NONE;
}

static int 
compress_blake2b (uint8_t *out, const uint8_t *blocks[], unsigned int blocks_to_comp)
{
  blake2b_state s;
  if (blake2b_init (&s, BLAKE_2B_BLOCK_SIZE))
    return ERROR_BLAKE_2B;

  for (unsigned int i = 0; i < blocks_to_comp; i++) {
    if (blake2b_update (&s, blocks[i], BLAKE_2B_BLOCK_SIZE))
      return ERROR_BLAKE_2B;
  }

  if (blake2b_final (&s, out, BLAKE_2B_BLOCK_SIZE))
      return ERROR_BLAKE_2B;

  return ERROR_NONE;
}

static int 
compress_sempira_2048 (uint8_t *out, const uint8_t *blocks[], unsigned int blocks_to_comp)
{
  if (blocks_to_comp != 1)
    return ERROR_SEMPIRA;

  memcpy (out, blocks[0], SEMPIRA_2048_BLOCK_SIZE);
  sempira2048_pipe (out, NULL, NULL, NULL, 1);

  return ERROR_NONE;
}

static int 
compress_echo (uint8_t *out, const uint8_t *blocks[], unsigned int blocks_to_comp)
{
  hashState s;
  
  if (Init (&s, 8*ECHO_BLOCK_SIZE))
    return ERROR_ECHO;

  for (unsigned int i = 0; i < blocks_to_comp; i++) {
    if (Update (&s, blocks[i], ECHO_BLOCK_SIZE))
      return ERROR_ECHO;
  }

  if (Final (&s, out))
      return ERROR_ECHO;

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

static int 
compress_hash (uint8_t *out, const uint8_t *blocks[], size_t blocks_to_comp,
    enum comp_method comp)
{
  // TODO: Insert hash metadata (block index and node index) at
  // each compression function call to prevent state reuse.
  switch (comp) {
    case COMP__KECCAK_1600:
      return compress_keccak (out, blocks, blocks_to_comp);
    case COMP__ARGON:
      return compress_argon (out, blocks, blocks_to_comp);
    case COMP__BLAKE_2B:
      return compress_blake2b (out, blocks, blocks_to_comp);
    case COMP__SHA_512:
      return compress_sha512 (out, blocks, blocks_to_comp);
    case COMP__ECHO:
      return compress_echo (out, blocks, blocks_to_comp);
    case COMP__SEMPIRA_2048:
      return compress_sempira_2048 (out, blocks, blocks_to_comp);
    case COMP__END:
      break;
  }

  return ERROR_INVALID_COMPRESSION_METHOD;
}


static int 
compress_xor (uint8_t *out, const uint8_t *blocks[], size_t blocks_to_comp,
    enum comp_method comp)
{
  // XOR
  const uint16_t block_size = compress_block_size (comp);
  uint8_t buf[block_size];
  memset (buf, 0, sizeof (buf));
  for (unsigned int i = 1; i < blocks_to_comp; i++) {
    xor_block_self (buf, blocks[i], block_size);
  }

  const uint8_t *to_hash[2] = { blocks[0], buf };
  return compress_hash (out, to_hash, 2, comp);
}

uint16_t
compress_block_size (enum comp_method comp)
{
  switch (comp)
  {
    case COMP__KECCAK_1600:
      return KECCAK_1600_BLOCK_SIZE;
    case COMP__BLAKE_2B:
      return BLAKE_2B_BLOCK_SIZE;
    case COMP__ARGON:
      return ARGON2_BLOCK_SIZE;
    case COMP__SHA_512:
      return SHA512_DIGEST_LENGTH;
    case COMP__ECHO:
      return ECHO_BLOCK_SIZE;
    case COMP__SEMPIRA_2048:
      return SEMPIRA_2048_BLOCK_SIZE;
    case COMP__END:
      break;
  }

  return 0;
}

