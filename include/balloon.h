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

enum comp_method {
  COMP__KECCAK_1600,
  COMP__ARGON,
  COMP__BLAKE_2B,
  COMP__SHA_512,
  COMP__ECHO,
  COMP__SEMPIRA_2048,

  COMP__END
};

enum comb_method {
  COMB__HASH,
  COMB__XOR,

  COMB__END
};

struct comp_options {
  enum comp_method comp;
  enum comb_method comb;
};

enum mix_method {
  MIX__BALLOON_SINGLE_BUFFER,
  MIX__BALLOON_DOUBLE_BUFFER,
  MIX__BALLOON_DOUBLE_BUFFER_PAR,
  MIX__BALLOON_DOUBLE_BUFFER_PIPE,

  MIX__ARGON2_UNIFORM,
  MIX__CATENA_BRG,
  MIX__SCRYPT,

  MIX__END
};

struct balloon_options {
  uint64_t m_cost;
  uint64_t t_cost;

  uint16_t n_threads;

  // Degree of expander graph used in the construction.
  // TODO: Make sure that this number is big enough to 
  //       get the expansion we need.
  // TODO: Set n_neighbors automatically and give warning
  //       when user overrides the default.
  uint8_t n_neighbors;
  struct comp_options comp_opts;
  enum mix_method mix;
};

int BalloonHash (void *out, size_t outlen, 
    const void *in, size_t inlen, 
    const void *salt, size_t saltlen, 
    struct balloon_options *opts);

#endif /* __BALLOON_H__ */

