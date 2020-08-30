/*
 * Copyright (c) 2016, Henry Corrigan-Gibbs
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

#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include <stdint.h>
#include <stdlib.h>

uint64_t
bytes_to_littleend_uint64 (const uint8_t *bytes, size_t n_bytes);

uint32_t
bytes_to_littleend_uint32 (const uint8_t *bytes, size_t n_bytes);

void
uint64_to_littleend_bytes (uint8_t *bytes, size_t n_bytes, uint64_t value);

void
uint32_to_littleend_bytes (uint8_t *bytes, size_t n_bytes, uint32_t value);

#endif
