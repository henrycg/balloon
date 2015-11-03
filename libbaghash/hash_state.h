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

#ifndef __HASH_STATE_H__
#define __HASH_STATE_H__

#include <stddef.h>

#include "bitstream.h"
#include "options.h"

#define UNUSED __attribute__ ((unused))

struct hash_state;

typedef int func_hash_state_init (struct hash_state *s, struct baghash_options *opts);
typedef int func_hash_state_free (struct hash_state *s);
typedef int func_hash_state_fill (struct hash_state *s, const void *in, size_t inlen,
    const void *salt, size_t saltlen);
typedef int func_hash_state_mix (struct hash_state *s);
typedef int func_hash_state_extract (struct hash_state *s, void *out, size_t outlen);

struct hash_state {
  uint64_t n_blocks;
  uint16_t block_size;
  bool has_mixed;
  uint8_t *buffer;
  struct bitstream bstream;
  struct baghash_options *opts;

  func_hash_state_init *f_init;
  func_hash_state_free *f_free;
  func_hash_state_fill *f_fill;
  func_hash_state_mix *f_mix;
  func_hash_state_extract *f_extract;

  void *extra_data;
};


int hash_state_init (struct hash_state *s, struct baghash_options *opts,
    const void *salt, size_t saltlen);

int hash_state_free (struct hash_state *s);

int hash_state_fill (struct hash_state *s, const void *in, size_t inlen,
    const void *salt, size_t saltlen);

int hash_state_mix (struct hash_state *s);

int hash_state_extract (struct hash_state *s, void *out, size_t outlen);


int fill_bytes_from_strings (struct hash_state *s, 
    uint8_t *block_start, size_t bytes_to_fill,
    const void *in, size_t inlen,
    const void *salt, size_t saltlen);

void *
block_index (const struct hash_state *s, size_t i);

void *
block_last (const struct hash_state *s);

#endif
