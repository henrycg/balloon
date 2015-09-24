#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include <stdbool.h>
#include <openssl/aes.h>
#include <openssl/sha.h>

// How many bytes to generate with AES at each invocation.
#define BITSTREAM_BUF_SIZE 4096

struct bitstream {
  // All of the below are directly passed to OpenSSL
  bool initialized;
  SHA256_CTX c;
  AES_KEY key;
  unsigned char init_vector[AES_BLOCK_SIZE];
  unsigned char ecount_buf[AES_BLOCK_SIZE]; 
  unsigned int n_bytes_encrypted;

  unsigned char *genp;
  unsigned char generated[BITSTREAM_BUF_SIZE];
};


int bitstream_init (struct bitstream *b);

int bitstream_seed_add (struct bitstream *b, const void *seed, size_t seedlen);

int bitstream_seed_finalize (struct bitstream *b);

// Buffer "out" MUST be zeroed before calling this method.
int bitstream_fill_buffer (struct bitstream *b, void *out, size_t outlen);

#endif
