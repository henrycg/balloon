
#include <openssl/aes.h>
#include <openssl/sha.h>

#include <stdlib.h>
#include <string.h>

#include "errors.h"
#include "hash_state.h"

int hash_state_init (struct hash_state *s, size_t n_blocks, size_t block_size)
{
  s->n_blocks = n_blocks;
  s->block_size = block_size;
  s->buffer = calloc (n_blocks, block_size);

  return (!s->buffer) ?  ERROR_NONE : ERROR_MALLOC;
}

void hash_state_free (struct hash_state *s)
{
  free (s->buffer);
}

int hash_state_fill (struct hash_state *s, 
    const void *in, size_t inlen,
    const void *salt, size_t saltlen)
{
  // Hash in and salt into a 256-bit AES key
  SHA256_CTX c;
  unsigned char key_bytes[SHA256_DIGEST_LENGTH];

  if (!SHA256_Init(&c))
    return ERROR_OPENSSL_HASH;

  if (!SHA256_Update(&c, in, inlen))
    return ERROR_OPENSSL_HASH;
  if (!SHA256_Update(&c, salt, saltlen))
    return ERROR_OPENSSL_HASH;

  if (!SHA256_Final(key_bytes, &c))
    return ERROR_OPENSSL_HASH;

  const size_t buflen = s->n_blocks * s->block_size;

  AES_KEY key; 
  if (!AES_set_encrypt_key(key_bytes, 256, &key))
    return ERROR_OPENSSL_AES;

  unsigned char ivec[AES_BLOCK_SIZE];
  unsigned char ecount_buf[AES_BLOCK_SIZE];
  unsigned int num = 0;

  memset(ivec, 0, AES_BLOCK_SIZE);
  memset(ecount_buf, 0, AES_BLOCK_SIZE);
  AES_ctr128_encrypt (s->buffer, s->buffer, buflen, &key, ivec, ecount_buf, &num);

  return ERROR_NONE;
}
