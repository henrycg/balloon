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


// TODO: Add license text to every file

#include <assert.h>
#include <baghash.h>

#include "constants.h"
#include "errors.h"
#include "hash_state.h"
#include "options.h"

static int validate_parameters (size_t outlen, size_t inlen, size_t saltlen);

int 
BagHash (void *out, size_t outlen, 
    const void *in, size_t inlen, 
    const void *salt, size_t saltlen, 
    struct baghash_options *opts)
{
  int error;

  // Sanity check parameters
  if (!out || !in || !salt)
    return ERROR_NULL_POINTER;

  if ((error = validate_parameters (outlen, inlen, saltlen)))
    return error;

  if ((error = options_validate (opts)))
    return error;

  struct hash_state state;
  if ((error = hash_state_init (&state, opts, salt, saltlen)))
    return error;

  // Fill buffer of m_cost blocks with pseudo-random stuff derived
  // from the password and salt.
  if ((error = hash_state_fill (&state, in, inlen, salt, saltlen)))
    return error;


  // Mix the buffer t_cost times
  for (unsigned int i = 0; i < opts->t_cost; i++) {
    if ((error = hash_state_mix (&state)))
      return error;
  }
 
  // Extract the output from the hash state
  if ((error = hash_state_extract (&state, out, outlen)))
    return error;
 
  hash_state_free (&state);

  return ERROR_NONE;
}

static int
validate_parameters (size_t outlen, size_t inlen, size_t saltlen)
{
  if (outlen < OUTLEN_MIN)
    return ERROR_OUTLEN_TOO_SMALL;
  if (outlen >= OUTLEN_MAX)
    return ERROR_OUTLEN_TOO_BIG;

  if (inlen < INLEN_MIN)
    return ERROR_INLEN_TOO_SMALL;
  if (inlen >= INLEN_MAX)
    return ERROR_INLEN_TOO_BIG;

  if (saltlen < SALTLEN_MIN)
    return ERROR_SALTLEN_TOO_SMALL;
  if (saltlen >= SALTLEN_MAX)
    return ERROR_SALTLEN_TOO_BIG;

  return 0;
}


