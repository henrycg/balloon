#ifndef __COMPRESS_H__
#define __COMPRESS_H__

// These are parameters of the Keccak hash function.
// The sum of these two must be equal to 1600. 
#define KECCAK_RATE 1344ull
#define KECCAK_CAPACITY 256

int compress (unsigned char *out, unsigned const char *blocks[],
    unsigned int n_blocks, size_t block_size);

#endif
