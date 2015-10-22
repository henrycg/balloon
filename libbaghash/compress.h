#ifndef __COMPRESS_H__
#define __COMPRESS_H__

#include <baghash.h>
#include <stdint.h>
#include "constants.h"
#include "hash_state.h"

int compress (uint8_t *out, const uint8_t *blocks[], 
    size_t blocks_to_comp, struct comp_options *opts);

uint16_t compress_block_size (enum comp_method comp);

#endif
