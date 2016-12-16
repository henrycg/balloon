
#include <balloon.h>
#include <stdio.h>

#include "errors.h"
#include "parse.h"

int balloon_internal (void *out, size_t outlen, 
    const void *in, size_t inlen, 
    const void *salt, size_t saltlen, 
    struct balloon_options *opts);

#define SALT_LEN (16)
#define HASH_LEN (32)

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
  uint8_t hash_bytes[HASH_LEN];
  uint8_t salt_bytes[SALT_LEN];

  if ((error = get_salt (salt_bytes)) != ERROR_NONE)
    return error;

  if ((error = balloon_internal (hash_bytes, HASH_LEN, 
      passwd, passwd_len, 
      salt_bytes, SALT_LEN, 
      opt)) != ERROR_NONE)
    return error; 

  if ((error = write_blob (out, BLOB_LEN,
      hash_bytes, HASH_LEN, 
      salt_bytes, SALT_LEN,
      opt->s_cost, opt->t_cost, opt->n_threads)))
    return error;

  return ERROR_NONE;
}

/*
int 
Balloon_Verify (uint8_t blob[BLOB_LEN], struct balloon_options *opt, 
    const uint8_t *passwd, size_t passwd_len)
{

}

*/
