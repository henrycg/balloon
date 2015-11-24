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


#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "bitstream.h"
#include "constants.h"
#include "compress.h"
#include "errors.h"
#include "hash_state.h"
#include "hash_state_argon2.h"
#include "hash_state_double.h"
#include "hash_state_double_par.h"
#include "hash_state_single.h"

#define MIN(a, b) ((a < b) ? (a) : (b))

void *
block_index (const struct hash_state *s, size_t i)
{
  return s->buffer + (s->block_size * i);
}

void *
block_last (const struct hash_state *s)
{
  return block_index (s, s->n_blocks - 1);
}

static int
init_func_pointers (struct hash_state *s)
{
  switch (s->opts->mix) {
    case MIX__BALLOON_SINGLE_BUFFER:
      s->f_init = hash_state_single_init;
      s->f_free = hash_state_single_free;
      s->f_fill = hash_state_single_fill;
      s->f_mix = hash_state_single_mix;
      s->f_extract = hash_state_single_extract;
      break;

    case MIX__BALLOON_DOUBLE_BUFFER:
      s->f_init = hash_state_double_init;
      s->f_free = hash_state_double_free;
      s->f_fill = hash_state_double_fill;
      s->f_mix = hash_state_double_mix;
      s->f_extract = hash_state_double_extract;
      break;

    case MIX__BALLOON_DOUBLE_BUFFER_PAR:
      s->f_init = hash_state_double_par_init;
      s->f_free = hash_state_double_par_free;
      s->f_fill = hash_state_double_par_fill;
      s->f_mix = hash_state_double_par_mix;
      s->f_extract = hash_state_double_par_extract;
      break;

    case MIX__ARGON2_UNIFORM:
      s->f_init = hash_state_argon2_init;
      s->f_free = hash_state_argon2_free;
      s->f_fill = hash_state_argon2_fill;
      s->f_mix = hash_state_argon2_mix;
      s->f_extract = hash_state_argon2_extract;
      break;

    default:
      return ERROR_INVALID_MIX_METHOD;
  } 

  return ERROR_NONE;
}

int 
hash_state_init (struct hash_state *s, struct balloon_options *opts,
    const void *salt, size_t saltlen)
{
  s->n_blocks = options_n_blocks (opts);

  // Force number of blocks to be even
  if (s->n_blocks % 2 != 0) s->n_blocks++;

  s->has_mixed = false;
  s->block_size = compress_block_size (opts->comp_opts.comp);
  s->opts = opts;

  // TODO: Make sure this multiplication doesn't overflow (or use 
  // calloc or realloc)
  s->buffer = malloc (s->n_blocks * s->block_size);

  int error;
  if ((error = init_func_pointers (s)))
    return error;
  if ((error = bitstream_init_with_seed (&s->bstream, salt, saltlen)))
    return error;
  if ((error = s->f_init (s, opts)))
    return error;

  return (s->buffer) ? ERROR_NONE : ERROR_MALLOC;
}

int
hash_state_free (struct hash_state *s)
{
  int error;
  if ((error = bitstream_free (&s->bstream)))
    return error;
  if ((error = s->f_free (s)))
    return error;
  free (s->buffer);
  return ERROR_NONE;
}

int 
hash_state_fill (struct hash_state *s, 
    const void *in, size_t inlen,
    const void *salt, size_t saltlen)
{
  return s->f_fill (s, in, inlen, salt, saltlen);
}

int 
hash_state_mix (struct hash_state *s)
{
  s->has_mixed = true;
  return s->f_mix (s);
}

int 
hash_state_extract (struct hash_state *s, void *out, size_t outlen)
{
  if (!s->has_mixed)
    return ERROR_CANNOT_EXTRACT_BEFORE_MIX;
  return s->f_extract (s, out, outlen);
}

int 
fill_bytes_from_strings (__attribute__  ((unused)) struct hash_state *s, 
    uint8_t *block_start, size_t bytes_to_fill,
    const void *in, size_t inlen,
    const void *salt, size_t saltlen)
{
  int error;
  struct bitstream bits;
  if ((error = bitstream_init (&bits)))
    return error;
  if ((error = bitstream_seed_add (&bits, in, inlen)))
    return error;
  if ((error = bitstream_seed_add (&bits, salt, saltlen)))
    return error;
  if ((error = bitstream_seed_finalize (&bits)))
    return error;
  if ((error = bitstream_fill_buffer (&bits, block_start, bytes_to_fill)))
    return error;
  if ((error = bitstream_free (&bits)))
    return error;

  return ERROR_NONE;
}

