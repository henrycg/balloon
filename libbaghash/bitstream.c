#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bitstream.h"
#include "constants.h"
#include "errors.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

static int refresh_state (struct bitstream *b);
static int bytes_required (size_t val);
static int bits_in_int (size_t val);
static size_t bytes_to_int (const unsigned char *bytes, int n_bytes);
static int generate_few_bytes (struct bitstream *b, unsigned char *out, size_t outlen);
static int bitstream_rand_byte (struct bitstream *b, unsigned char *out);

int
bitstream_init (struct bitstream *b)
{
  if (!SHA256_Init(&b->c))
    return ERROR_OPENSSL_HASH;
  b->initialized = false;

  EVP_CIPHER_CTX_init (&b->ctx);

  if (!(b->zeros = malloc (BITSTREAM_BUF_SIZE * sizeof (unsigned char))))
    return ERROR_MALLOC;
  if (!(b->generated = malloc (BITSTREAM_BUF_SIZE * sizeof (unsigned char))))
    return ERROR_MALLOC;

  memset (b->zeros, 0, BITSTREAM_BUF_SIZE);

  b->genp = NULL;
  b->n_refreshes = 0;

  return ERROR_NONE;
}

int
bitstream_free (struct bitstream *b)
{
  unsigned char out[AES_BLOCK_SIZE];
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
  unsigned char key_bytes[SHA256_DIGEST_LENGTH];

  if (!SHA256_Final(key_bytes, &b->c))
    return ERROR_OPENSSL_HASH;

  // TODO: Make the IV depend on the salt?
  unsigned char iv[AES_BLOCK_SIZE];
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
    unsigned char *buf = malloc (aligned_buf * sizeof (unsigned char));
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
bitstream_rand_int (struct bitstream *b, size_t *out, size_t max)
{
  int error; 
  const int n_bytes = bytes_required (max);
  const int bits = bits_in_int (max);
  unsigned char buf[n_bytes];
  size_t retval;

  do {
    if ((error = generate_few_bytes (b, buf, n_bytes)))
      return error;

    retval = bytes_to_int (buf, n_bytes);
    retval %= (1 << bits);
  } while (retval >= max);

  *out = retval;
  return ERROR_NONE;
}

static int
bitstream_rand_byte (struct bitstream *b, unsigned char *out)
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
generate_few_bytes (struct bitstream *b, unsigned char *out, size_t outlen)
{
  int error;
  for (int i = 0; i < outlen; i++) {
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
static int
bytes_required (size_t val)
{
  if (val < 2) return 0;

  int bytes_reqd;
  for (bytes_reqd = 0; bytes_reqd < 8; bytes_reqd++) {
    if (val < (1 << (8*bytes_reqd))) break;
  }

  return bytes_reqd;
}

static size_t
bytes_to_int (const unsigned char *bytes, int n_bytes)
{
  size_t out = 0;
  for (int i = 0; i < n_bytes; i++) {
    out <<= 8;
    out |= bytes[i];
  }

  return out;
}

static int 
bits_in_int (size_t val)
{
    int ret = 0;

    while (val) {
      val >>= 1;
      ret++;
    }

    return ret;
}
