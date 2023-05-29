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
#include <stdio.h>
#include <string.h>

#include "bitstream.h"
#include "constants.h"
#include "encode.h"
#include "errors.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

int
bitstream_init (struct bitstream *b)
{
  if (!SHA256_Init(&b->c))
    return ERROR_OPENSSL_HASH;
  b->initialized = false;

  b->ctx = EVP_CIPHER_CTX_new ();

  if (!(b->zeros = malloc (BITSTREAM_BUF_SIZE * sizeof (uint8_t))))
    return ERROR_MALLOC;
  memset (b->zeros, 0, BITSTREAM_BUF_SIZE);

  return ERROR_NONE;
}

int
bitstream_free (struct bitstream *b)
{
  uint8_t out[AES_BLOCK_SIZE];
  int outl;
  if (!EVP_EncryptFinal_ex (b->ctx, out, &outl))
    return ERROR_OPENSSL_AES;

  EVP_CIPHER_CTX_free (b->ctx);

  free (b->zeros);

  return ERROR_NONE;
}

int 
bitstream_seed_add (struct bitstream *b, const void *seed, size_t seedlen)
{
  if (b->initialized)
    return ERROR_BITSTREAM_FINALIZED;

  if (!SHA256_Update(&b->c, seed, seedlen))
    return ERROR_OPENSSL_HASH;
  return ERROR_NONE;
}

int 
bitstream_seed_finalize (struct bitstream *b)
{
  // Hash in and salt into a 128-bit AES key
  uint8_t key_bytes[SHA256_DIGEST_LENGTH];

  if (!SHA256_Final(key_bytes, &b->c))
    return ERROR_OPENSSL_HASH;

  uint8_t iv[AES_BLOCK_SIZE];
  memset (iv, 0, AES_BLOCK_SIZE);

  //printf("Key: ");
  //for(int i=0; i<32; i++) 
    //printf("%02x,", key_bytes[i]);
  //  printf("%d,", key_bytes[i]);
  //puts("");

  if (!EVP_CIPHER_CTX_set_padding (b->ctx, 1))
    return ERROR_OPENSSL_AES;

  if (!EVP_EncryptInit (b->ctx, EVP_aes_128_ctr (), key_bytes, iv))
    return ERROR_OPENSSL_AES;

  b->initialized = true;
  return ERROR_NONE;
}

static int 
encrypt_partial (struct bitstream *b, void *outp, int to_encrypt)
{
  int encl;
  // Encrypt directly into the output buffer
  if (!EVP_EncryptUpdate (b->ctx, outp, &encl, b->zeros, to_encrypt))
    return ERROR_OPENSSL_AES;

  return ERROR_NONE;
}

int 
bitstream_fill_buffer (struct bitstream *b, void *out, size_t outlen)
{
  int error;
  if (!b->initialized)
    return ERROR_BITSTREAM_UNINITIALIZED;

  size_t total = 0;
  while (total < outlen) {
    const int to_encrypt = MIN(outlen - total, BITSTREAM_BUF_SIZE);
    if ((error = encrypt_partial (b, out + total, to_encrypt)))
      return error;

    total += to_encrypt;
  }

  assert (((size_t)total) == outlen);
  return ERROR_NONE;
}


int
bitstream_rand_uint64 (struct bitstream *b, uint64_t *out)
{
  int error; 
  const int n_bytes = 8;
  uint8_t buf[n_bytes];

  if ((error = bitstream_fill_buffer (b, buf, n_bytes)))
    return error;

  *out = bytes_to_littleend_uint64 (buf, n_bytes);
  return ERROR_NONE;
}

