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

#ifndef __HASH_STATE_CATENA_H__
#define __HASH_STATE_CATENA_H__

#include <stddef.h>

#include "bitstream.h"
#include "options.h"

struct hash_state;

uint64_t catena_nearest_power_of_two (uint64_t in, int *n_bits);
uint64_t catena_reverse_bits (uint64_t in, int n_bits);
uint64_t catena_butterfly (uint64_t in, int n_bits, int layer);

int hash_state_catena_init (struct hash_state *s, struct balloon_options *opts);

int hash_state_catena_free (struct hash_state *s);

int hash_state_catena_brg_extract (struct hash_state *s, void *out, size_t outlen);

int hash_state_catena_dbg_mix (struct hash_state *s);

#endif
