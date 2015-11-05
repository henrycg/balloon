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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitstream.h"
#include "constants.h"
#include "compress.h"
#include "errors.h"
#include "hash_state.h"
#include "hash_state_double.h"
#include "xor.h"

struct double_par_data {
  uint8_t *src;
  uint8_t *dst;

  uint8_t *join_block;
  struct bitstream *bstreams;
};

struct thread_data {
  struct hash_state *s;
  uint16_t thread_idx;
  bool mix;
  struct bitstream pstream;
};

static void *mix_partial (void *in);
static int run_threads (struct hash_state *s, bool mix,
    const void *in, size_t inlen, const void *salt, size_t saltlen);

int 
hash_state_double_par_init (struct hash_state *s, UNUSED struct baghash_options *opts)
{
  struct double_par_data *data = malloc (sizeof (*data));
  if (!data)
    return ERROR_MALLOC;

  data->src = block_index (s, 0);
  data->dst = block_index (s, s->n_blocks / 2);
  data->bstreams = malloc (sizeof (struct bitstream) * s->opts->n_threads);
  if (!data->bstreams)
    return ERROR_MALLOC;
  data->join_block = malloc (s->block_size);
  if (!data->join_block)
    return ERROR_MALLOC;

  memset (data->join_block, 0, s->block_size);

  int error;
  const int seedlen = 32;
  for (uint16_t i = 0; i < s->opts->n_threads; i++) {
    uint8_t seed[seedlen];
    if ((error = bitstream_fill_buffer (&s->bstream, seed, seedlen)))
      return error;

    if ((error = bitstream_init_with_seed (&data->bstreams[i], seed, seedlen)))
      return error;
  }
 
  s->extra_data = data;
  return ERROR_NONE; 
}

int
hash_state_double_par_free (struct hash_state *s)
{
  struct double_par_data *data = (struct double_par_data *)s->extra_data;
  
  int error;
  for (int i = 0; i < s->opts->n_threads; i++) {
    if ((error = bitstream_free (&data->bstreams[i])))
      return error;
  }
  free (data->join_block);
  free (data->bstreams);
  free (s->extra_data);
  return ERROR_NONE;
}


int 
hash_state_double_par_fill (struct hash_state *s, 
    const void *in, size_t inlen,
    const void *salt, size_t saltlen)
{
  return run_threads (s, false, in, inlen, salt, saltlen);
}

static uint8_t *
rel_block_index (struct hash_state *s, uint8_t *buf, size_t idx)
{
  return buf + (s->block_size * idx);
}

static size_t 
thread_start (uint16_t n_threads, uint16_t thread_idx, size_t blocks_per_buf)
{
  const size_t blocks_per_thread = blocks_per_buf / n_threads;
  return (thread_idx * blocks_per_thread);
}

static size_t 
thread_end (uint16_t n_threads, uint16_t thread_idx, size_t blocks_per_buf)
{
  const size_t blocks_per_thread = blocks_per_buf / n_threads;
  const size_t start = thread_start (n_threads, thread_idx, blocks_per_buf);
  return (thread_idx == (n_threads - 1)) ? blocks_per_buf : start + blocks_per_thread;
}

static void *
mix_partial (void *in)
{
  int error;

  struct thread_data *d = in; 
  struct hash_state *s = d->s;

  struct double_par_data *data = (struct double_par_data *)s->extra_data;
  const size_t blocks_per_buf = s->n_blocks / 2; 
  const size_t start = thread_start (s->opts->n_threads, d->thread_idx, blocks_per_buf);
  const size_t end = thread_end (s->opts->n_threads, d->thread_idx, blocks_per_buf);
  const bool is_xor = s->opts->comp_opts.comb == COMB__XOR;

  if (d->mix) {
    //printf("[%d] Start: %d, End: %d\n", (int)thread_idx, (int)start, (int)end);
    for (size_t i = start; i < end; i++) {
      void *cur_block = rel_block_index (s, data->dst, i);

      size_t n_neighbors = s->opts->n_neighbors;
      // TODO: Check if sorting the neighbors before accessing them improves
      // the performance at all.
      uint64_t neighbors[n_neighbors];
      if ((error = bitstream_rand_ints_nodup (&data->bstreams[d->thread_idx], neighbors, 
            &n_neighbors, n_neighbors, blocks_per_buf)))
        return (void *)(uint64_t)error;

      const size_t n_blocks_to_hash = is_xor ? n_neighbors : n_neighbors + 1;
      const uint8_t *blocks[n_blocks_to_hash];
      
      // If this is the first block of the segment, hash with the join block.
      // Otherwise, hash with the previous block in the buffer.
      if (!is_xor)
        blocks[n_neighbors] = (i == start) ? data->join_block : rel_block_index (s, data->src, i - 1);

      // For each block, pick random neighbors
      for (size_t n = 0; n < n_neighbors; n++) { 
        // Get next neighbor
        blocks[n] = rel_block_index (s, data->src, neighbors[n]);
      }

      // Hash value of neighbors into temp buffer.
      if ((error = compress (cur_block, blocks, n_blocks_to_hash, &s->opts->comp_opts)))
        return (void *)(uint64_t)error;
    }
  } else {
    const size_t blen = (end - start) * s->block_size;
    uint8_t *bufp = rel_block_index (s, data->src, start);
    if ((error = bitstream_fill_buffer (&d->pstream, bufp, blen))) {
      return (void *)(uint64_t)error;
    }
  }

  return (void *)ERROR_NONE;
}

static int
run_threads (struct hash_state *s, bool mix,
    const void *in, size_t inlen,
    const void *salt, size_t saltlen)
{
  struct thread_data t_params[s->opts->n_threads];
  pthread_t threads[s->opts->n_threads];

  // Start each thread
  int error;
  for (uint16_t i = 0; i < s->opts->n_threads; i++) {
    t_params[i].s = s;
    t_params[i].thread_idx = i;
    t_params[i].mix = mix;

    if (!mix) {
      uint8_t bytes[2];
      bytes[0] = i & 0xff;
      bytes[1] = (i >> 8) & 0xff;
      if ((error = bitstream_init (&t_params[i].pstream)))
        return error;
      if ((error = bitstream_seed_add (&t_params[i].pstream, in, inlen)))
        return error;
      if ((error = bitstream_seed_add (&t_params[i].pstream, salt, saltlen)))
        return error;
      if ((error = bitstream_seed_add (&t_params[i].pstream, bytes, 2)))
        return error;
      if ((error = bitstream_seed_finalize (&t_params[i].pstream)))
        return error;
    }

    if (pthread_create (&threads[i], NULL, mix_partial, (void *)&t_params[i]))
      return ERROR_PTHREAD;
  }

  // Join all of the threads
  for (uint16_t i = 0; i < s->opts->n_threads; i++) {
    void *ret;
    if (pthread_join (threads[i], &ret))
      return ERROR_PTHREAD;
    if (ret != (void*)ERROR_NONE)
      return (uint64_t)ret;

    if (!mix) {
      if ((error = bitstream_free (&t_params[i].pstream)))
        return error;
    }
  }

  return ERROR_NONE;
}

int 
hash_state_double_par_mix (struct hash_state *s)
{
  int error;
  if ((error = run_threads (s, true, NULL, 0, NULL, 0)))
    return error;

  struct double_par_data *data = (struct double_par_data *)s->extra_data;

  // Update the join block by hashing together the last blocks of each 
  // thread's segment.
  const size_t n_blocks = s->opts->n_threads + 1;
  const uint8_t *blocks[n_blocks];
  blocks[0] = data->join_block;
  for (uint16_t t = 0; t < s->opts->n_threads; t++) {
    const size_t end = thread_end (s->opts->n_threads, t, s->n_blocks / 2);
    blocks[t + 1] = rel_block_index (s, data->dst, end - 1);
  }

  if ((error = compress (data->join_block, blocks, n_blocks, &s->opts->comp_opts)))
    return error;
  
  uint8_t *tmp = data->src;
  data->src = data->dst;
  data->dst = tmp; 

  return ERROR_NONE;
}


int 
hash_state_double_par_extract (struct hash_state *s, void *out, size_t outlen)
{
  const struct double_par_data *data = (struct double_par_data *)s->extra_data;
  const uint8_t *bufp = data->join_block;

  if (s->opts->comp_opts.comb == COMB__XOR) {
    // If we are using one of the XOR combining method, XOR the contents
    // of the last buffer together and output that.
    uint8_t tmp[s->block_size];
    memset (tmp, 0, s->block_size);
    for (size_t i = 0; i < s->n_blocks / 2; i++) {
      xor_block (tmp, tmp, rel_block_index (s, data->src, i), s->block_size);
    }
    bufp = tmp;
  } 

  return fill_bytes_from_strings (s, out, outlen, bufp, s->block_size, NULL, 0);
}

