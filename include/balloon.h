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


#ifndef __BALLOON_H__
#define __BALLOON_H__

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

struct balloon_options {
  uint32_t s_cost;
  uint32_t t_cost;
  uint32_t n_threads;
};

/* Maximum password blob length (in bytes). */
#define BLOB_LEN (160)

int Balloon_Hash (char out[BLOB_LEN], struct balloon_options *opt, 
    const char *passwd, size_t passwd_len);

int Balloon_Verify (const char blob[BLOB_LEN], 
    const char *passwd, size_t passwd_len);

#endif /* __BALLOON_H__ */

