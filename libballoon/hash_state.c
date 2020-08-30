/*
 * Copyright (c) 2015-2016, Henry Corrigan-Gibbs
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
#include <stdlib.h>
#include <string.h>

#include "bitstream.h"
#include "constants.h"
#include "compress.h"
#include "encode.h"
#include "errors.h"
#include "hash_state.h"

#define MIN(a, b) ((a < b) ? (a) : (b))

void *
block_index (const struct hash_state *s, size_t i)
{
  return s->buffer + (BLOCK_SIZE * i);
}

static uint64_t
options_n_blocks (const struct balloon_options *opts)
{
  const uint32_t bsize = BLOCK_SIZE;
  // Covert s_cost into KB
  uint64_t ret = (opts->s_cost * 1024) / bsize;
  return (ret < BLOCKS_MIN) ? BLOCKS_MIN : ret;
}


void *
block_last (const struct hash_state *s)
{
  return block_index (s, s->n_blocks - 1);
}


int 
hash_state_init (struct hash_state *s, const struct balloon_options *opts,
    const uint8_t salt[SALT_LEN])
{
  s->counter = 0;
  s->n_blocks = options_n_blocks (opts);

  // Force number of blocks to be even
  if (s->n_blocks % 2 != 0) s->n_blocks++;

  s->has_mixed = false;
  s->opts = opts;

  // TODO: Make sure that this multiplication doesn't overflow 
  // (or use calloc or realloc)
  s->buffer = malloc (s->n_blocks * BLOCK_SIZE);

  int error;
  if ((error = bitstream_init (&s->bstream)))
    return error;
  if ((error = bitstream_seed_add (&s->bstream, salt, SALT_LEN)))
    return error;

  uint8_t temp[4];
  uint32_to_littleend_bytes (temp, 4, opts->s_cost);
  if ((error = bitstream_seed_add(&s->bstream, temp, 4)))
    return error;

  uint32_to_littleend_bytes (temp, 4, opts->t_cost);
  if ((error = bitstream_seed_add(&s->bstream, temp, 4)))
    return error;

  uint32_to_littleend_bytes (temp, 4, opts->n_threads);
  if ((error = bitstream_seed_add(&s->bstream, temp, 4)))
    return error;

  if ((error = bitstream_seed_finalize (&s->bstream)))
    return error;

  return (s->buffer) ? ERROR_NONE : ERROR_MALLOC;
}

int
hash_state_free (struct hash_state *s)
{
  int error;
  if ((error = bitstream_free (&s->bstream)))
    return error;
  free (s->buffer);
  return ERROR_NONE;
}

int 
hash_state_fill (struct hash_state *s, 
    const uint8_t salt[SALT_LEN],
    const uint8_t *in, size_t inlen)
{
  int error;
  uint8_t temp[8];

  // Hash salt and password into 0-th block
  SHA256_CTX c;
  if (!SHA256_Init(&c))
    return ERROR_OPENSSL_HASH;
  uint64_to_littleend_bytes (temp, 8, s->counter);
  if (!SHA256_Update(&c, temp, 8))
    return ERROR_OPENSSL_HASH;
  if (!SHA256_Update(&c, salt, SALT_LEN))
    return ERROR_OPENSSL_HASH;
  if (!SHA256_Update(&c, in, inlen))
    return ERROR_OPENSSL_HASH;
  uint32_to_littleend_bytes (temp, 4, s->opts->s_cost);
  if (!SHA256_Update(&c, temp, 4))
    return ERROR_OPENSSL_HASH;
  uint32_to_littleend_bytes (temp, 4, s->opts->t_cost);
  if (!SHA256_Update(&c, temp, 4))
    return ERROR_OPENSSL_HASH;
  uint32_to_littleend_bytes (temp, 4, s->opts->n_threads);
  if (!SHA256_Update(&c, temp, 4))
    return ERROR_OPENSSL_HASH;
  if (!SHA256_Final(s->buffer, &c))
    return ERROR_OPENSSL_HASH;

  s->counter++;

  if ((error = expand (&s->counter, s->buffer, s->n_blocks)))
    return error;

  return ERROR_NONE;
}

int 
hash_state_mix (struct hash_state *s)
{
  int error;
  uint64_t neighbor;
  
  // Simplest design: hash in place with one buffer
  for (size_t i = 0; i < s->n_blocks; i++) {
    uint8_t *cur_block = block_index (s, i);

    const uint8_t *blocks[2+N_NEIGHBORS];

    // Hash in the previous block (or the last block if this is
    // the first block of the buffer).
    const uint8_t *prev_block = i ? cur_block - BLOCK_SIZE : block_last (s);

    blocks[0] = prev_block;
    blocks[1] = cur_block;

    // For each block, pick random neighbors
    for (size_t n = 2; n < 2+N_NEIGHBORS; n++) { 
      // Get next neighbor
      if ((error = bitstream_rand_uint64 (&s->bstream, &neighbor)))
        return error;
      blocks[n] = block_index (s, neighbor % s->n_blocks);
    }

    // Hash value of neighbors into temp buffer.
    if ((error = compress (&s->counter, cur_block, blocks, 2+N_NEIGHBORS)))
      return error;
  }
  s->has_mixed = true;
  return ERROR_NONE;
}

int 
hash_state_extract (const struct hash_state *s, uint8_t out[BLOCK_SIZE])
{
  if (!s->has_mixed)
    return ERROR_CANNOT_EXTRACT_BEFORE_MIX;

  // Return bytes derived from the last block of the buffer.
  uint8_t *b = block_last (s);
  memcpy ((char *)out, (const char *)b, BLOCK_SIZE);
  return ERROR_NONE;
}

