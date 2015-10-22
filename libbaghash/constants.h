#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#define OUTLEN_MIN 16ull
#define OUTLEN_MAX 128ull

#define INLEN_MIN 4ull
#define INLEN_MAX (1ull<<20)

#define SALTLEN_MIN 4ull
#define SALTLEN_MAX (1ull<<20)

// Key length (in bits) for the AES-CTR cipher used
// to fill up the buffers initially.
#define AES_CTR_KEY_LEN 256

// Time cost (in number of passes over memory)
#define TCOST_MIN 1ull
// Memory cost (roughly in number of bytes)
#define MCOST_MIN (1024ull)
// Minimum number of blocks to use
#define BLOCKS_MIN (32ull)

// The product of these two values must be less
// than 2^64 to avoid integer overflow in size_t.
#define MCOST_MAX (1ull << 48)
#define KECCAK_1600_BLOCK_SIZE (168)

// These are parameters of the Keccak hash function.
// The sum of these two must be equal to 1600. 
#define KECCAK_RATE (8 * KECCAK_1600_BLOCK_SIZE)
#define KECCAK_CAPACITY (1600 - KECCAK_RATE)

#define BLAKE_2B_BLOCK_SIZE (64)

#define ECHO_BLOCK_SIZE (64)

#endif

