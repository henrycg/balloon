
#include <baghash.h>

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

  return ERROR_NONE;
}

size_t
options_block_size (struct baghash_options *opts)
{
  if (opts->comp == COMP__KECCAK_1600)
    return 100;

  return 0;
}

size_t
options_n_blocks (struct baghash_options *opts)
{
  const size_t bsize = options_block_size (opts);
  size_t ret = opts->m_cost / bsize;
  return (ret < BLOCKS_MIN) ? BLOCKS_MIN : ret;
}

