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
  // All of the below are directly passed to OpenSSL
  bool initialized;
  SHA256_CTX c;
  EVP_CIPHER_CTX ctx;

  uint8_t *zeros;
  uint8_t *generated;

  uint8_t *genp;
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
int bitstream_rand_int (struct bitstream *b, uint64_t *out, uint64_t max);

int bitstream_rand_ints (struct bitstream *b, uint64_t *outs, size_t outlen, 
  uint64_t max, bool distinct);

/**
 * Get a single char.
 */
int bitstream_rand_byte (struct bitstream *b, uint8_t *out);

#endif
