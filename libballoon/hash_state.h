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

#define UNUSED __attribute__ ((unused))

struct hash_state;

struct hash_state {
  uint64_t n_blocks;
  bool has_mixed;
  uint8_t *buffer;
  struct bitstream bstream;
  const struct balloon_options *opts;
};


int hash_state_init (struct hash_state *s, const struct balloon_options *opts,
    const uint8_t salt[SALT_LEN]);

int hash_state_free (struct hash_state *s);

int hash_state_fill (struct hash_state *s, 
    const uint8_t salt[SALT_LEN], const uint8_t *in, size_t inlen);

int hash_state_mix (struct hash_state *s);

int hash_state_extract (const struct hash_state *s, uint8_t out[BLOCK_SIZE]);

void *
block_index (const struct hash_state *s, size_t i);

void *
block_last (const struct hash_state *s);

#endif
