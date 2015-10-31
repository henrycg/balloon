
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matgen/matrix_gen.h"

#include "bitstream.h"
#include "constants.h"
#include "compress.h"
#include "errors.h"
#include "hash_state.h"
#include "hash_state_double.h"

struct double_par_data {
  uint8_t *src;
  uint8_t *dst;

  struct matrix_generator **matgens;
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
  if (s->opts->comp_opts.comb == COMB__XOR) {
    if (s->opts->n_neighbors) {
      fprintf(stderr, "Warning: Using non-standard n_neighbors\n"); 
    } 
  }

  data->matgens = malloc (sizeof (struct matrix_generator *) * s->opts->n_threads);
  if (!data->matgens)
    return ERROR_MALLOC;
  data->bstreams = malloc (sizeof (struct bitstream) * s->opts->n_threads);
  if (!data->bstreams)
    return ERROR_MALLOC;

  int error;
  const int seedlen = 32;
  for (uint16_t i = 0; i < s->opts->n_threads; i++) {
    uint8_t seed[seedlen];
    if ((error = bitstream_fill_buffer (&s->bstream, seed, seedlen)))
      return error;

    if ((error = bitstream_init_with_seed (&data->bstreams[i], seed, seedlen)))
      return error;

    data->matgens[i] = matrix_generator_init (&data->bstreams[i], s->n_blocks / 2);
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

    matrix_generator_free (data->matgens[i]);
  }
  
  free (data->bstreams);
  free (data->matgens);
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

static void *
mix_partial (void *in)
{
  int error;

  struct thread_data *d = in; 
  struct hash_state *s = d->s;
  const uint16_t thread_idx = d->thread_idx;

  struct double_par_data *data = (struct double_par_data *)s->extra_data;
  const bool distinct_neighbs = (s->opts->comp_opts.comb == COMB__XOR);
  const size_t blocks_per_buf = s->n_blocks / 2; 
  size_t row_neighbors = s->opts->n_neighbors;

  const uint16_t n_threads = s->opts->n_threads;
  const size_t blocks_per_thread = blocks_per_buf / n_threads;
  const size_t start = (thread_idx * blocks_per_thread);
  const size_t end = (thread_idx == (n_threads - 1)) ? (blocks_per_buf - 1) : start + blocks_per_thread;

  if (d->mix) {
    //printf("[%d] Start: %d, End: %d\n", (int)thread_idx, (int)start, (int)end);
    for (size_t i = start; i < end; i++) {
      void *cur_block = rel_block_index (s, data->dst, i);

      // Only use the fancy distribution if the user has not specified
      // a number of neighbors to use.
      if (s->opts->comp_opts.comb == COMB__XOR && !s->opts->n_neighbors) {
        if ((error = matrix_generator_row_weight (data->matgens[thread_idx], &row_neighbors)))
          return (void *)(uint64_t)error;
      }

      const uint8_t *blocks[row_neighbors];

      // TODO: Check if sorting the neighbors before accessing them improves
      // the performance at all.
      uint64_t neighbors[row_neighbors];

      if ((error = bitstream_rand_ints (&data->bstreams[thread_idx], neighbors, 
            row_neighbors, blocks_per_buf, distinct_neighbs)))
        return (void *)(uint64_t)error;

      // For each block, pick random neighbors
      for (size_t n = 0; n < row_neighbors; n++) { 
        // Get next neighbor
        blocks[n] = rel_block_index (s, data->src, neighbors[n]);
      }

      // Hash value of neighbors into temp buffer.
      if ((error = compress (cur_block, blocks, row_neighbors, &s->opts->comp_opts)))
        return (void *)(uint64_t)error;
    }
  } else {
    const size_t blen = (end - start) * s->block_size;
    uint8_t *bufp = rel_block_index (s, data->src, start);
    if ((error = bitstream_fill_buffer (&d->pstream, bufp, blen)))
      return (void *)(uint64_t)error;
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

  // Swap the role of the src and dst buffers.
  // At the entry of the mix function, we are always copying from src to dst.
  uint8_t *tmp = data->src;
  data->src = data->dst;
  data->dst = tmp; 

  return ERROR_NONE;
}


