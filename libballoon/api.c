
#include <balloon.h>
#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "errors.h"
#include "parse.h"

int balloon_internal (uint8_t out[BLOCK_SIZE],
    const uint8_t salt[SALT_LEN], 
    const void *in, size_t inlen, 
    struct balloon_options *opts);


static int
get_salt (uint8_t *salt)
{
  FILE *fp = fopen("/dev/urandom", "r");
  if (!fp) return ERROR_URANDOM;

  int ch;
  for (int i = 0; i < SALT_LEN; i++) {
    ch = getc (fp);
    if (ch < 0) return ERROR_URANDOM;
    salt[i] = (uint8_t)ch;
  } 

  if (fclose(fp))
    return ERROR_URANDOM; 

  return ERROR_NONE;
}

int 
Balloon_Hash (char out[BLOB_LEN], struct balloon_options *opt, 
    const char *passwd, size_t passwd_len)
{
  int error;
  uint8_t hash_bytes[BLOCK_SIZE];
  uint8_t salt_bytes[SALT_LEN];

  if ((error = get_salt (salt_bytes)) != ERROR_NONE)
    return error;

  if ((error = balloon_internal (hash_bytes, salt_bytes,  passwd, passwd_len, opt)) != ERROR_NONE)
    return error; 

  if ((error = write_blob (out, BLOB_LEN,
      salt_bytes, 
      hash_bytes, BLOCK_SIZE, 
      opt->s_cost, opt->t_cost, opt->n_threads)))
    return error;

  return ERROR_NONE;
}

int 
Balloon_Verify (char blob[BLOB_LEN], const char *passwd, size_t passwd_len)
{

  int error;
  uint8_t hash[BLOCK_SIZE]; 
  uint8_t salt[SALT_LEN]; 

  struct balloon_options opt;
 
  if ((error = read_blob (blob, BLOB_LEN,
      salt, 
      hash, BLOCK_SIZE, 
      &opt.s_cost, &opt.t_cost, &opt.n_threads)))
    return error;

  uint8_t hash2[BLOCK_SIZE]; 
  if ((error = balloon_internal (hash2, salt, passwd, passwd_len, &opt)) != ERROR_NONE)
    return error; 

  // Return ERROR_NONE on success.
  return !memcmp ((char *)hash, (char *)hash2, BLOCK_SIZE) ? ERROR_NONE : ERROR_HASH_MISMATCH;
}

