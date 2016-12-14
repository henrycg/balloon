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

#include <limits.h>

#define OUTLEN_MIN 16ull
#define OUTLEN_MAX 128ull

#define N_NEIGHBORS (3)

#define INLEN_MAX (1ull<<20)
#define SALTLEN_MAX (1ull<<20)

// Key length (in bits) for the AES-CTR cipher used
// to fill up the buffers initially.
#define AES_CTR_KEY_LEN 256

// Time cost (in number of passes over memory)
#define TCOST_MIN 1ull
// Memory cost (roughly in number of bytes)
#define MCOST_MIN (1024ull)
#define MCOST_MAX (UINT32_MAX)
// Minimum number of blocks to use
#define BLOCKS_MIN (32ull)

// Maximum number of threads
#define THREADS_MAX 255

#define BLOCK_SIZE (16)

#endif

