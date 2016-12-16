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
#include <pthread.h>
#include <string.h>

#include "constants.h"
#include "encode.h"
#include "errors.h"
#include "hash_state.h"

static int validate_parameters (size_t outlen, size_t inlen, size_t saltlen);
static void *balloon_worker (void *data);
static void xor (uint8_t *a, const uint8_t *b, size_t len);

struct worker_data {
  const struct balloon_options *opts;
  uint32_t thread_idx;

  uint8_t *out;
  size_t outlen;

  const uint8_t *in;
  size_t inlen;

  uint8_t *salt;
  size_t saltlen;

  int error;
};

static void
xor (uint8_t *a, const uint8_t *b, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    a[i] ^= b[i];
  }
}

void
worker_salt (uint8_t *tsalt, const uint8_t *salt, uint8_t saltlen, uint32_t worker_idx)
{
  // Add worker idx to last 32 bits of salt (as a little-endian integer)

  strncpy ((char *)tsalt, (const char *)salt, saltlen);
  uint8_t *last_bytes = tsalt + (saltlen - 4);
  uint32_t byteint = bytes_to_littleend_uint32(last_bytes, 4) + worker_idx; 

  uint8_t byte0 = byteint & 0xFF;
  //printf("0 = %x\n", byte0);
  last_bytes[0] = byte0;
  last_bytes[1] = (byteint & 0xFF00) >> 8;
  last_bytes[2] = (byteint & 0xFF0000) >> 16;
  last_bytes[3] = (byteint & 0xFF000000) >> 24;
}

static void *
balloon_worker(void *datap)
{
  struct worker_data *data = (struct worker_data *)datap;
  
  struct hash_state state;
  if ((data->error = hash_state_init (&state, data->opts, data->salt, data->saltlen)))
    return NULL; 

  // Fill buffer of s_cost blocks with pseudo-random stuff derived
  // from the password and salt.
  if ((data->error = hash_state_fill (&state, data->in, data->inlen, data->salt, data->saltlen)))
    return NULL;

  // Mix the buffer t_cost times
  for (unsigned int i = 0; i < data->opts->t_cost; i++) {
    if ((data->error = hash_state_mix (&state)))
      return NULL;
  }
 
  // Extract the output from the hash state
  if ((data->error = hash_state_extract (&state, data->out, data->outlen)))
    return NULL;
 
  hash_state_free (&state);

  return NULL;
}

int 
balloon_internal (void *out, size_t outlen, 
    const void *in, size_t inlen, 
    const void *salt, size_t saltlen, 
    struct balloon_options *opts)
{
  int error;

  // Sanity check parameters
  if (!out || !in || !salt)
    return ERROR_NULL_POINTER;

  if ((error = validate_parameters (outlen, inlen, saltlen)))
    return error;

  if ((error = options_validate (opts)))
    return error;

  struct worker_data thread_data[opts->n_threads]; 
  pthread_t thread_id[opts->n_threads]; 
  for (uint32_t t = 0; t < opts->n_threads; t++) {
    uint8_t *tout = malloc (outlen);
    uint8_t *tsalt = malloc (saltlen);
    if (!tout) return ERROR_MALLOC;

    worker_salt (tsalt, salt, saltlen, t);
    printf("salt: ");
    for(unsigned int i=0; i<saltlen;i++) {
      printf("%x", tsalt[i]);
    }
    printf("\n");

    thread_data[t] = (struct worker_data){ 
      .opts = opts,
      .thread_idx = t,
      .out = tout,
      .outlen = outlen, 
      .in = in, 
      .inlen = inlen, 
      .salt = tsalt, 
      .saltlen = saltlen, 
      .error = ERROR_NONE
    };

    error = pthread_create(&thread_id[t], NULL, balloon_worker, (void *) &thread_data[t]);
    if (error) 
      return ERROR_PTHREAD;
  }
printf("done\n");

  for (uint32_t t = 0; t < opts->n_threads; t++) {
    if (pthread_join(thread_id[t], NULL))
      return ERROR_PTHREAD;

    if (thread_data[t].error != ERROR_NONE)
      return thread_data[t].error;

    xor (out, thread_data[t].out, outlen);

    free (thread_data[t].out);
    free (thread_data[t].salt);
  }

  return ERROR_NONE;
}

static int
validate_parameters (size_t outlen, size_t inlen, size_t saltlen)
{
  if (outlen < OUTLEN_MIN)
    return ERROR_OUTLEN_TOO_SMALL;
  if (outlen >= OUTLEN_MAX)
    return ERROR_OUTLEN_TOO_BIG;

  if (inlen >= INLEN_MAX)
    return ERROR_INLEN_TOO_BIG;

  if (saltlen >= SALTLEN_MAX)
    return ERROR_SALTLEN_TOO_BIG;

  return 0;
}


