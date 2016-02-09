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


#include <balloon.h>

#include "blake2b/argon2-core.h"
#include "compress.h"
#include "constants.h"
#include "errors.h"
#include "options.h"

int 
options_validate (struct balloon_options *opts)
{
  // TODO: Make sure choice of compression function, mix function,
  // and other parameters is sane.

  if (opts->t_cost < TCOST_MIN)
    opts->t_cost = TCOST_MIN;
  if (opts->m_cost < MCOST_MIN)
    opts->m_cost = MCOST_MIN;
  if (opts->m_cost >= MCOST_MAX)
    return ERROR_MCOST_TOO_BIG;

  if (!opts->n_neighbors) {
    if (opts->comp_opts.comb != COMB__XOR)
      return ERROR_NO_NEIGHBORS;
  }

  if (opts->n_threads > THREADS_MAX)
    return ERROR_NTHREADS_TOO_BIG;

  if (opts->mix != MIX__BALLOON_DOUBLE_BUFFER_PAR && opts->n_threads > 1) {
    return ERROR_INCOMPATIBLE_OPTIONS;
  }

  if (opts->mix == MIX__BALLOON_DOUBLE_BUFFER_PIPE) {
    if (opts->comp_opts.comp != COMP__SEMPIRA_2048 ||
      opts->comp_opts.comb != COMB__XOR) {
        fprintf (stderr, "Mix method double-pipe must use options -x and -c sempira2048\n");
        return ERROR_INCOMPATIBLE_OPTIONS;
    }
  }

  if (opts->mix != MIX__BALLOON_DOUBLE_BUFFER_PIPE &&
        opts->comp_opts.comp == COMP__SEMPIRA_2048) {
    fprintf (stderr, "-c sempira2048 only compatible with -m double-pipe\n");
    return ERROR_INCOMPATIBLE_OPTIONS;
  }
  

  if (opts->mix == MIX__SCRYPT && opts->t_cost > 1)
    return ERROR_TCOST_TOO_BIG;
  if (opts->mix == MIX__SCRYPT && opts->n_threads > 1)
    return ERROR_TCOST_TOO_BIG;

  const uint64_t n_blocks = options_n_blocks (opts);
  const uint16_t block_size = compress_block_size (opts->comp_opts.comp);
  if (n_blocks > UINT64_MAX / block_size)
    return ERROR_MCOST_TOO_BIG;

  if (opts->comp_opts.comb == COMB__XOR) {
    if (opts->mix != MIX__BALLOON_DOUBLE_BUFFER && 
        opts->mix != MIX__BALLOON_DOUBLE_BUFFER_PAR &&
        opts->mix != MIX__BALLOON_DOUBLE_BUFFER_PIPE)
      return ERROR_INCOMPATIBLE_OPTIONS;
  }

  return ERROR_NONE;
}

#if 0
static uint8_t
number_of_neighbors_single (const struct balloon_options *opts) 
{
  // We need sets of size N/4 to expand to sets of size 5N/8.
  uint8_t ret; 
  const uint64_t n_blocks = options_n_blocks (opts);
  if (n_blocks > 2048)
    ret = 13;
  else if (n_blocks > 256)
    ret = 15;
  else if (n_blocks > 128) 
    ret = 17;
  else if (n_blocks > 64) 
    ret = 22;
  else if (n_blocks > 32) 
    ret = 32;
  else if (n_blocks > 16) 
    ret = 51;
  else 
    ret = 80;
  return ret;
}

static unsigned int
number_of_neighbors_double (const struct balloon_options *opts) 
{
  // If N is the total number of blocks in the buffer, each 
  // of the two buffers has size n = N/2.
  // We need to pick the number of neighbors such that sets 
  // of n/4 to expand to sets of size n/2.
  // We choose these values to make the probability of failure
  // less than 2^{-80}.

  unsigned int ret; 
  const size_t n_blocks = options_n_blocks (opts);
  if (n_blocks > 2048)
    ret = 13;
  else if (n_blocks > 512)
    ret = 15;
  else if (n_blocks > 256)
    ret = 18;
  else if (n_blocks > 128) 
    ret = 23;
  else if (n_blocks > 64) 
    ret = 33;
  else 
    ret = 40;
  return ret;
}
#endif

uint8_t
options_n_neighbors (const struct balloon_options *opts)
{
  // Note: to keep performance evaluation statistics consistent,
  // fix these values to relatively conservative ones. In practice,
  // we would reduce the degree for large block sizes.
  if (opts->comp_opts.comb == COMB__XOR)
    return 10;

  switch (opts->mix) {
    case MIX__BALLOON_SINGLE_BUFFER:
      return 20; // number_of_neighbors_single (opts);
    case MIX__BALLOON_DOUBLE_BUFFER_PAR:
    case MIX__BALLOON_DOUBLE_BUFFER:
      return 20; // number_of_neighbors_double (opts);
    case MIX__ARGON2_UNIFORM:
      return 1;
    case MIX__SCRYPT:
      return 1;
    default: 
      return 0;
  }
}

uint64_t
options_n_blocks (const struct balloon_options *opts)
{
  const uint16_t bsize = compress_block_size (opts->comp_opts.comp);
  uint64_t ret = opts->m_cost / bsize;
  return (ret < BLOCKS_MIN) ? BLOCKS_MIN : ret;
}

