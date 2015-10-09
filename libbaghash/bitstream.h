#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include <stdbool.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

// How many bytes to generate with AES at each invocation.
#define BITSTREAM_BUF_SIZE ((1 << 10) * (AES_BLOCK_SIZE))

struct bitstream {
  // All of the below are directly passed to OpenSSL
  bool initialized;
  SHA256_CTX c;
  EVP_CIPHER_CTX ctx;

  unsigned char *zeros;
  unsigned char *generated;

  unsigned char *genp;
  unsigned int n_refreshes;
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
 * Return a random integer in the "out" parameter in the range
 * [0, max) -- exclusive of max.
 */
int bitstream_rand_int (struct bitstream *b, size_t *out, size_t max);

int bitstream_rand_ints (struct bitstream *b, size_t *outs, size_t outlen, 
  size_t max, bool distinct);

/**
 * Get a single char.
 */
int bitstream_rand_byte (struct bitstream *b, unsigned char *out);

#endif
