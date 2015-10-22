
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "blake2b/argon2-core.h"
#include "blake2b/blake2.h"
// This both Blake2b and Keccak use this macro
#undef ALIGN
#include "keccak/KeccakSponge.h"

#include "compress.h"
#include "errors.h"

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

static void
xor_block (uint8_t *out, const uint8_t *blockA, const uint8_t *blockB,
  size_t block_size)
{
  for (size_t i = 0; i < block_size; i++) {
    out[i] = blockA[i] ^ blockB[i];
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
  Argon2FillBlock (out, blocks[0], (blocks_to_comp > 1 ? blocks[1] : out));
  for (unsigned int i = 2; i < blocks_to_comp; i++) {
    Argon2FillBlock (out, out, blocks[i]);
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
    xor_block (buf, buf, blocks[i], block_size);
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
    case COMP__END:
      break;
  }

  return 0;
}

