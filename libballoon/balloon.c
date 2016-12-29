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
#include <balloon.h>
#include <pthread.h>
#include <string.h>

#include "constants.h"
#include "encode.h"
#include "errors.h"
#include "hash_state.h"

static void *balloon_worker (void *data);
static void xor (uint8_t *a, const uint8_t *b, size_t len);

struct worker_data {
  const struct balloon_options *opts;
  uint32_t thread_idx;

  uint8_t out[BLOCK_SIZE];
  uint8_t salt[SALT_LEN];

  const uint8_t *in;
  size_t inlen;

  int error;
};

static void
xor (uint8_t *a, const uint8_t *b, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    a[i] ^= b[i];
  }
}

static int 
options_validate (struct balloon_options *opts)
{
  if (opts->t_cost < TCOST_MIN)
    return ERROR_TCOST_TOO_SMALL;
  if (opts->s_cost < SCOST_MIN)
    return ERROR_SCOST_TOO_SMALL;
  if (opts->s_cost >= SCOST_MAX)
    return ERROR_SCOST_TOO_BIG;

  if (opts->n_threads > THREADS_MAX)
    return ERROR_NTHREADS_TOO_BIG;

  return ERROR_NONE;
}



void
worker_salt (uint8_t tsalt[SALT_LEN], const uint8_t salt[SALT_LEN], uint32_t worker_idx)
{
  // Add worker idx to last 32 bits of salt (as a little-endian integer)

  strncpy ((char *)tsalt, (const char *)salt, SALT_LEN);
  uint32_t byteint = bytes_to_littleend_uint32(tsalt, 4) + worker_idx; 

  uint8_t byte0 = byteint & 0xFF;
  //printf("0 = %x\n", byte0);
  tsalt[0] = byte0;
  tsalt[1] = (byteint & 0xFF00) >> 8;
  tsalt[2] = (byteint & 0xFF0000) >> 16;
  tsalt[3] = (byteint & 0xFF000000) >> 24;
}

static void *
balloon_worker(void *datap)
{
  struct worker_data *data = (struct worker_data *)datap;
  
  struct hash_state state;
  if ((data->error = hash_state_init (&state, data->opts, data->salt)))
    return NULL; 

  // Fill buffer of s_cost blocks with pseudo-random stuff derived
  // from the password and salt.
  if ((data->error = hash_state_fill (&state, data->salt, data->in, data->inlen)))
    return NULL;

  // Mix the buffer t_cost times
  for (unsigned int i = 0; i < data->opts->t_cost; i++) {
    if ((data->error = hash_state_mix (&state)))
      return NULL;
  }
 
  // Extract the output from the hash state
  if ((data->error = hash_state_extract (&state, data->out)))
    return NULL;
 
  hash_state_free (&state);

  return NULL;
}

int 
balloon_internal (uint8_t out[BLOCK_SIZE], 
    const uint8_t salt[SALT_LEN],
    const void *in, size_t inlen, 
    struct balloon_options *opts)
{
  int error;

  // Sanity check parameters
  if (!out || !in || !salt)
    return ERROR_NULL_POINTER;

  if (inlen >= INLEN_MAX)
    return ERROR_INLEN_TOO_BIG;

  if ((error = options_validate (opts)))
    return error;

  struct worker_data thread_data[opts->n_threads]; 
  pthread_t thread_id[opts->n_threads]; 
  for (uint32_t t = 0; t < opts->n_threads; t++) {
    thread_data[t] = (struct worker_data){ 
      .opts = opts,
      .thread_idx = t,
      .in = in, 
      .inlen = inlen, 
      .error = ERROR_NONE
    };

    worker_salt (thread_data[t].salt, salt, t);
    printf ("salt: ");
    for (unsigned int i=0; i<SALT_LEN;i++) {
      printf ("%x", thread_data[t].salt[i]);
    }
    printf ("\n");

    error = pthread_create (&thread_id[t], NULL, balloon_worker, (void *) &thread_data[t]);
    if (error) 
      return ERROR_PTHREAD;
  }
  printf("done\n");

  bzero (out, BLOCK_SIZE);
  for (uint32_t t = 0; t < opts->n_threads; t++) {
    if (pthread_join(thread_id[t], NULL))
      return ERROR_PTHREAD;

    if (thread_data[t].error != ERROR_NONE)
      return thread_data[t].error;

    xor (out, thread_data[t].out, BLOCK_SIZE);
  }

  return ERROR_NONE;
}

