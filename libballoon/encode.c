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

#include "encode.h"

uint64_t
bytes_to_littleend_uint64 (const uint8_t *bytes, size_t n_bytes)
{
  if (n_bytes > 8) 
    n_bytes = 8;

  uint64_t out = 0;
  for (int i = n_bytes-1; i >= 0; i--) {
    out <<= 8;
    out |= bytes[i];
  }

  return out;
}

uint32_t
bytes_to_littleend_uint32 (const uint8_t *bytes, size_t n_bytes)
{
  uint64_t out_big = bytes_to_littleend_uint64 (bytes, n_bytes); 
  uint32_t out = out_big & 0xFFFFFFFF;
  return out;
}

void
uint64_to_littleend_bytes (uint8_t *bytes, size_t n_bytes, uint64_t value)
{
  for (size_t i = 0; i < n_bytes; i++) {
    bytes[i] = value & 0xFF;
    value >>= 8;
  }
}

void
uint32_to_littleend_bytes (uint8_t *bytes, size_t n_bytes, uint32_t value)
{
  for (size_t i = 0; i < n_bytes; i++) {
    bytes[i] = value & 0xFF;
    value >>= 8;
  }
}
