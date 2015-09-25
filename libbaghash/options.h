#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <baghash.h>

int options_validate (struct baghash_options *opt);

size_t options_n_blocks (struct baghash_options *opts);

size_t options_block_size (struct baghash_options *opts);

#endif
