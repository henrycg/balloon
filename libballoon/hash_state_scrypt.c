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

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "bitstream.h"
#include "constants.h"
#include "compress.h"
#include "errors.h"
#include "hash_state.h"

int 
hash_state_scrypt_init (UNUSED struct hash_state *s, UNUSED struct balloon_options *opts)
{
  return ERROR_NONE; 
}

int
hash_state_scrypt_free (UNUSED struct hash_state *s)
{
  return ERROR_NONE;
}

int 
hash_state_scrypt_fill (struct hash_state *s, 
    const void *in, size_t inlen,
    const void *salt, size_t saltlen)
{
  int error;
  // Fill first block of buffer.
  if ((error = fill_bytes_from_strings (s, s->buffer, s->block_size, in, inlen, salt, saltlen)))
    return error;

  uint8_t zeros[s->block_size];
  memset (zeros, 0, s->block_size);

  // block i = Hash (block i-1)
  for (uint64_t i = 1; i < s->n_blocks; i++) {
    const uint8_t *blocks[2];
    blocks[0] = block_index (s, i-1);
    blocks[1] = zeros;

    if ((error = compress (block_index (s, i), blocks, 2, &s->opts->comp_opts)))
      return error;
  }

  return ERROR_NONE;
}

int 
hash_state_scrypt_mix (UNUSED struct hash_state *s)
{
  // Scrypt doesn't have the notion of mixing 
  return ERROR_NONE;
}

static uint64_t
integrify (struct hash_state *s, uint8_t *block)
{
  uint64_t out;
  memcpy (&out, block, MIN(sizeof (uint64_t), s->block_size));
  return out;
}

int 
hash_state_scrypt_extract (struct hash_state *s, void *out, size_t outlen)
{
  int error;
  uint8_t accumulator[s->block_size];
  memcpy (accumulator, block_index (s, s->n_blocks - 1), s->block_size);

  const uint8_t *blocks[2];
  for (uint64_t i = 0; i < s->n_blocks; i++) {
    uint64_t next_block_idx = integrify (s, accumulator) % s->n_blocks;
    blocks[0] = accumulator;
    blocks[1] = block_index (s, next_block_idx);

    if ((error = compress (accumulator, blocks, 2, &s->opts->comp_opts)))
      return error;
  }

  // Return bytes derived from the last block of the buffer.
  return fill_bytes_from_strings (s, out, outlen, accumulator, s->block_size, NULL, 0);
}

