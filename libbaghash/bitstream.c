
#include <string.h>

#include "bitstream.h"
#include "constants.h"
#include "errors.h"

static int refresh_state (struct bitstream *b);
static int bytes_required (size_t val);
static int bits_in_int (size_t val);
static size_t bytes_to_int (const unsigned char *bytes, int n_bytes);

int
bitstream_init (struct bitstream *b)
{
  if (!SHA256_Init(&b->c))
    return ERROR_OPENSSL_HASH;
  b->initialized = false;
  memset(&b->init_vector, 0, AES_BLOCK_SIZE);
  memset(&b->ecount_buf, 0, AES_BLOCK_SIZE);
  b->n_bytes_encrypted = 0;
  b->genp = b->generated;

  return ERROR_NONE;
}

int 
bitstream_seed_add (struct bitstream *b, const void *seed, size_t seedlen)
{
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

  if (!AES_set_encrypt_key(key_bytes, AES_CTR_KEY_LEN, &b->key))
    return ERROR_OPENSSL_AES;

  b->initialized = true;
  return ERROR_NONE;
}

int 
bitstream_fill_buffer (struct bitstream *b, void *out, size_t outlen)
{
  if (!b->initialized)
    return ERROR_BITSTREAM_UNINITIALIZED;
  AES_ctr128_encrypt (out, out, outlen, &b->key, b->init_vector, 
    b->ecount_buf, &b->n_bytes_encrypted);
  return ERROR_NONE;
}

int
bitstream_rand_byte (struct bitstream *b, unsigned char *out)
{
  if (!b->initialized)
    return ERROR_BITSTREAM_UNINITIALIZED;

  if ((b->genp + sizeof (size_t)) >= b->generated) 
  {
    int error;
    if ((error = refresh_state (b)))
      return error;
  }

  *out = *b->genp;
  b->genp++;
  return ERROR_NONE;
}

int
bitstream_rand_int (struct bitstream *b, size_t *out, size_t max)
{
  int error; 
  const int n_bytes = bytes_required (max);
  unsigned char buf[n_bytes];

  const int bits = bits_in_int (max);
  size_t retval;
  do {
    for (int i = 0; i < n_bytes; i++) {
      if ((error = bitstream_rand_byte (b, &buf[i])))
        return error;
    }

    retval = bytes_to_int (buf, n_bytes);
    retval %= (1 << bits);
  } while (retval >= max);

  *out = retval;
  return ERROR_NONE;
}

static int
refresh_state (struct bitstream *b)
{
  memset (b->generated, 0, BITSTREAM_BUF_SIZE);
  return bitstream_fill_buffer (b, b->generated, BITSTREAM_BUF_SIZE);
}

static int
bytes_required (size_t val)
{
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
