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
#include "errors.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

static int refresh_state (struct bitstream *b);
static uint8_t bytes_required (uint64_t val);
static uint8_t bits_in_int (uint64_t val);
static uint64_t bytes_to_int (const uint8_t *bytes, size_t n_bytes);
static int generate_few_bytes (struct bitstream *b, uint8_t *out, size_t outlen);
static bool is_duplicate (uint64_t *outs, size_t idx);

int
bitstream_init (struct bitstream *b)
{
  if (!SHA256_Init(&b->c))
    return ERROR_OPENSSL_HASH;
  b->initialized = false;

  EVP_CIPHER_CTX_init (&b->ctx);

  if (!(b->zeros = malloc (BITSTREAM_BUF_SIZE * sizeof (uint8_t))))
    return ERROR_MALLOC;
  if (!(b->generated = malloc (BITSTREAM_BUF_SIZE * sizeof (uint8_t))))
    return ERROR_MALLOC;

  memset (b->zeros, 0, BITSTREAM_BUF_SIZE);

  b->genp = NULL;
  b->n_refreshes = 0;

  return ERROR_NONE;
}

int
bitstream_free (struct bitstream *b)
{
  uint8_t out[AES_BLOCK_SIZE];
  int outl;
  if (!EVP_EncryptFinal (&b->ctx, out, &outl))
    return ERROR_OPENSSL_AES;

  if (!EVP_CIPHER_CTX_cleanup (&b->ctx))
    return ERROR_OPENSSL_AES;

  free (b->zeros);
  free (b->generated);

  return ERROR_NONE;
}

int 
bitstream_init_with_seed (struct bitstream *b, const void *seed, size_t seedlen)
{
  int error;
  if ((error = bitstream_init (b)))
    return error;
  if ((error = bitstream_seed_add (b, seed, seedlen)))
    return error;
  if ((error = bitstream_seed_finalize (b)))
    return error;

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
  // Hash in and salt into a 256-bit AES key
  uint8_t key_bytes[SHA256_DIGEST_LENGTH];

  if (!SHA256_Final(key_bytes, &b->c))
    return ERROR_OPENSSL_HASH;

  // TODO: Make the IV depend on the salt?
  uint8_t iv[AES_BLOCK_SIZE];
  memset (iv, 0, AES_BLOCK_SIZE);

  if (!EVP_CIPHER_CTX_set_padding (&b->ctx, 0))
    return ERROR_OPENSSL_AES;

  if (!EVP_EncryptInit (&b->ctx, EVP_aes_256_cbc (), key_bytes, iv))
    return ERROR_OPENSSL_AES;

  b->initialized = true;
  return ERROR_NONE;
}

static int 
encrypt_partial (struct bitstream *b, void *outp, int to_encrypt)
{
  // TODO: Make sure encrypt the right block size
  int encl;
  if (to_encrypt % AES_BLOCK_SIZE == 0) {
    // Encrypt directly into the output buffer
    if (!EVP_EncryptUpdate (&b->ctx, outp, &encl, b->zeros, to_encrypt))
      return ERROR_OPENSSL_AES;
    assert (encl == to_encrypt);
  } else {
    // Round up to nearest block size
    const int aligned_buf = ((to_encrypt / AES_BLOCK_SIZE) + 1) * AES_BLOCK_SIZE;

    assert (to_encrypt < BITSTREAM_BUF_SIZE);
    assert (aligned_buf < BITSTREAM_BUF_SIZE);
    assert (aligned_buf >= to_encrypt);
    assert (aligned_buf % AES_BLOCK_SIZE == 0);

    // Encrypt to a temp buffer and copy the result over
    uint8_t *buf = malloc (aligned_buf * sizeof (uint8_t));
    if (!buf)
      return ERROR_MALLOC;
    if (!EVP_EncryptUpdate (&b->ctx, buf, &encl, b->zeros, aligned_buf))
      return ERROR_OPENSSL_AES;
    assert (encl > to_encrypt);
    memcpy (outp, buf, to_encrypt);
    free (buf);
  }

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
bitstream_rand_int (struct bitstream *b, uint64_t *out, uint64_t max)
{
  int error; 
  const int n_bytes = bytes_required (max);
  const uint8_t bits = bits_in_int (max);
  uint8_t buf[n_bytes];
  uint64_t retval;

  do {
    if ((error = generate_few_bytes (b, buf, n_bytes)))
      return error;

    retval = bytes_to_int (buf, n_bytes);
    retval %= (1 << bits);
  } while (retval >= max);

  *out = retval;
  return ERROR_NONE;
}

static bool 
is_duplicate (uint64_t *outs, size_t idx)
{
  // TODO: Use a sorted list here to speed up the search.
  for (size_t i = 0; i < idx; i++) {
    if (outs[i] == outs[idx]) return true;
  }

  return false;
}

int 
bitstream_rand_ints (struct bitstream *b, uint64_t *outs, size_t outlen, 
  uint64_t max, bool distinct)
{
  if (distinct && max < outlen)
    return ERROR_BITSTREAM_MAX_TOO_SMALL;

  int error;
  for (size_t i = 0; i < outlen; i++) {
    do {
    if ((error =  bitstream_rand_int (b, &outs[i], max)))
      return error;
    } while (distinct && is_duplicate (outs, i));
  }

  return ERROR_NONE;
}

int
bitstream_rand_byte (struct bitstream *b, uint8_t *out)
{
  if (!b->initialized)
    return ERROR_BITSTREAM_UNINITIALIZED;

  if (!b->genp || (b->genp + sizeof (size_t)) >= (b->generated + BITSTREAM_BUF_SIZE)) 
  {
    int error;
    if ((error = refresh_state (b)))
      return error;
  }

  *out = *(b->genp);
  b->genp++;
  return ERROR_NONE;
}

static int
generate_few_bytes (struct bitstream *b, uint8_t *out, size_t outlen)
{
  int error;
  for (size_t i = 0; i < outlen; i++) {
    if ((error = bitstream_rand_byte (b, &out[i])))
      return error;
  }

  return ERROR_NONE;
}

static int
refresh_state (struct bitstream *b)
{
  //printf("refresh\n");
  b->genp = b->generated;
  b->n_refreshes++;
  return bitstream_fill_buffer (b, b->generated, BITSTREAM_BUF_SIZE);
}


/**
 * The number of random bytes required to generate a random number
 * in the range [0, max). This should be zero for when max == 0 or
 * max == 1. 
 */
static uint8_t
bytes_required (uint64_t val)
{
  if (val < 2) return 0;

  uint8_t bytes_reqd;
  uint64_t one = 1;
  for (bytes_reqd = 0; bytes_reqd < 8; bytes_reqd++) {
    if (val < (one << (8*bytes_reqd))) break;
  }

  return bytes_reqd;
}

static uint64_t
bytes_to_int (const uint8_t *bytes, size_t n_bytes)
{
  uint64_t out = 0;
  for (size_t i = 0; i < n_bytes; i++) {
    out <<= 8;
    out |= bytes[i];
  }

  return out;
}

static uint8_t
bits_in_int (uint64_t val)
{
    uint8_t ret = 0;

    while (val) {
      val >>= 1;
      ret++;
    }

    return ret;
}
