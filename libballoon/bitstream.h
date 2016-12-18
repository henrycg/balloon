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

#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include <stdbool.h>
#include <stdint.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

// How many bytes to generate with AES at each invocation.
#define BITSTREAM_BUF_SIZE ((32) * (AES_BLOCK_SIZE))

struct bitstream {
  bool initialized;
  uint8_t *zeros;

  // All of the below are directly passed to OpenSSL
  SHA256_CTX c;
  EVP_CIPHER_CTX ctx;
};


int bitstream_init (struct bitstream *b);

int bitstream_free (struct bitstream *b);

int bitstream_init_with_seed (struct bitstream *b, const void *seed, size_t seedlen);

int bitstream_seed_add (struct bitstream *b, const void *seed, size_t seedlen);

int bitstream_seed_finalize (struct bitstream *b);

/** 
 * This function will zero out the "out" buffer before filling
 * it with pseudo-random bytes.
 */
int bitstream_fill_buffer (struct bitstream *b, void *out, size_t outlen);

/** 
 * Return a random uint64.
 */
int bitstream_rand_uint64 (struct bitstream *b, uint64_t *out);

/**
 * Get a single char.
 */
int bitstream_rand_byte (struct bitstream *b, uint8_t *out);

#endif
