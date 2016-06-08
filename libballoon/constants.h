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

// Maximum number of threads
#define THREADS_MAX 255

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

#define SIMPIRA_2048_BLOCK_SIZE (2048)

#endif

