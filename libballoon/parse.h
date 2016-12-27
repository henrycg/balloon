/*
 * Copyright (c) 2016, Henry Corrigan-Gibbs
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

#ifndef __PARSE_H__
#define __PARSE_H__

#include <inttypes.h>
#include <stdio.h>

int write_blob (char *blob, size_t bloblen,
      const uint8_t salt[SALT_LEN], 
      const uint8_t *out, size_t outlen,
      uint32_t s_cost, uint32_t t_cost, uint32_t n_threads);

int read_blob (const char *blob, size_t bloblen,
      uint8_t salt[SALT_LEN], uint8_t *out, size_t outlen,
      uint32_t *s_cost, uint32_t *t_cost, uint32_t *n_threads);

size_t n_tokens (const char *str, size_t strlen, uint8_t delim);

/**
 * WARNING: 
 *  - tokens must have size n_tokens(str)
 *  - str will be overwritten be a MUTABLE string
 *  - tokens returns pointers into str where the tokens begin 
 */
void tokenize (char **tokens, char *str, uint8_t delim);
int int_parse(const char *intstr, uint32_t *intp);
int parse_options (const char *optstr, size_t optlen,
  uint32_t *s_cost, uint32_t *t_cost, uint32_t *n_threads);

#endif

