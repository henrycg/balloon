#ifndef __COMPRESS_H__
#define __COMPRESS_H__

#include "constants.h"
#include "hash_state.h"

// These are parameters of the Keccak hash function.
// The sum of these two must be equal to 1600. 
#define KECCAK_RATE (8 * KECCAK_1600_BLOCK_SIZE)
#define KECCAK_CAPACITY (1600 - KECCAK_RATE)


int compress(unsigned char *out, unsigned const char *blocks[], unsigned int blocks_to_comp,
    enum comp_method comp);

int compress_xor(unsigned char *out, unsigned const char *blocks[], unsigned int blocks_to_comp,
    enum comp_method comp);

size_t compress_block_size (enum comp_method comp);

#endif
