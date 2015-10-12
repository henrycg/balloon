
#include <baghash.h>

#include "blake2b/argon2-core.h"
#include "compress.h"
#include "constants.h"
#include "errors.h"
#include "options.h"

int 
options_validate (struct baghash_options *opts)
{
  // TODO: Make sure choice of compression function, mix function,
  // and other parameters is sane.

  if (opts->t_cost < TCOST_MIN)
    opts->t_cost = TCOST_MIN;
  if (opts->m_cost < MCOST_MIN)
    opts->m_cost = MCOST_MIN;
  if (opts->m_cost >= MCOST_MAX)
    return ERROR_MCOST_TOO_BIG;

  if (!opts->n_neighbors)
    return ERROR_NO_NEIGHBORS;

  const uint64_t n_blocks = options_n_blocks (opts);
  const uint16_t block_size = compress_block_size (opts->comp_opts.comp);
  if (n_blocks > UINT64_MAX / block_size)
    return ERROR_MCOST_TOO_BIG;

  if (opts->mix != MIX__BAGHASH_DOUBLE_BUFFER && 
      opts->comp_opts.comb == COMB__XOR)
    return ERROR_INCOMPATIBLE_OPTIONS;

  return ERROR_NONE;
}

static uint8_t
number_of_neighbors_single (const struct baghash_options *opts) 
{
  // We need sets of size N/4 to expand to sets of size 5N/8.
  uint8_t ret; 
  const uint64_t n_blocks = options_n_blocks (opts);
  if (n_blocks > 2048)
    ret = 13;
  else if (n_blocks > 1024)
    ret = 15;
  else if (n_blocks > 512)
    ret = 14;
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
number_of_neighbors_double (const struct baghash_options *opts) 
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
  else if (n_blocks > 1024)
    ret = 15;
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

uint8_t
options_n_neighbors (const struct baghash_options *opts)
{
  switch (opts->mix) {
    case MIX__BAGHASH_SINGLE_BUFFER:
      return number_of_neighbors_single (opts);
    case MIX__BAGHASH_DOUBLE_BUFFER:
      return number_of_neighbors_double (opts);
    case MIX__ARGON2_UNIFORM:
      return 1;
    default: 
      return 0;
  }
}

uint64_t
options_n_blocks (const struct baghash_options *opts)
{
  const uint16_t bsize = compress_block_size (opts->comp_opts.comp);
  uint64_t ret = opts->m_cost / bsize;
  return (ret < BLOCKS_MIN) ? BLOCKS_MIN : ret;
}

