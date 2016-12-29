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

#define N_NEIGHBORS (3)

#define SALT_LEN (16)
#define INLEN_MAX (1ull<<20)

// Time cost (in number of passes over memory)
#define TCOST_MIN 1ull
// Space/memory cost (roughly in number of bytes)
#define SCOST_MIN (1)
#define SCOST_MAX (UINT32_MAX)
// Minimum number of blocks to use
#define BLOCKS_MIN (32ull)

// Maximum number of threads
#define THREADS_MAX 4096

#define BLOCK_SIZE (32)

#endif

