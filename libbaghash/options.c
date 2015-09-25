
#include <baghash.h>

#include "blake2b/argon2-core.h"
#include "compress.h"
#include "constants.h"
#include "errors.h"
#include "options.h"

int 
options_validate (struct baghash_options *opts)
{
  if (opts->t_cost < TCOST_MIN)
    opts->t_cost = TCOST_MIN;
  if (opts->m_cost < MCOST_MIN)
    opts->m_cost = MCOST_MIN;
  if (opts->m_cost >= MCOST_MAX)
    return ERROR_MCOST_TOO_BIG;

  if (!opts->n_neighbors)
    return ERROR_NO_NEIGHBORS;

  if (options_n_blocks (opts) > (1ull << 31) ||
      compress_block_size (opts->comp) > (1ull << 31) )
    return ERROR_MCOST_TOO_BIG;

  return ERROR_NONE;
}


size_t
options_n_blocks (struct baghash_options *opts)
{
  const size_t bsize = compress_block_size (opts->comp);
  size_t ret = opts->m_cost / bsize;
  return (ret < BLOCKS_MIN) ? BLOCKS_MIN : ret;
}

