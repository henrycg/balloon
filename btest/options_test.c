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


#include <balloon.h>

#include "mutest.h"

#include "libballoon/balloon.c"
#include "libballoon/constants.h"
#include "libballoon/errors.h"
#include "libballoon/options.h"

void 
mu_test_validate_parameters__outlen () 
{
  size_t outlen;
  size_t inlen;
  size_t saltlen;

  outlen = 32;
  inlen = 1024;
  saltlen = 32;
  mu_check ( validate_parameters (outlen, inlen, saltlen) == 0 );

  outlen = 0;
  mu_check ( validate_parameters (outlen, inlen, saltlen) == ERROR_OUTLEN_TOO_SMALL );

  outlen = OUTLEN_MIN - 1;
  mu_check ( validate_parameters (outlen, inlen, saltlen) == ERROR_OUTLEN_TOO_SMALL );

  outlen = 1 << 24;
  mu_check ( validate_parameters (outlen, inlen, saltlen) == ERROR_OUTLEN_TOO_BIG);

  outlen = OUTLEN_MAX;
  mu_check ( validate_parameters (outlen, inlen, saltlen) == ERROR_OUTLEN_TOO_BIG);
}


void 
mu_test_validate_parameters__inlen () 
{
  size_t outlen;
  size_t inlen;
  size_t saltlen;

  outlen = 32;
  inlen = 1024;
  saltlen = 32;
  mu_check ( validate_parameters (outlen, inlen, saltlen) == 0 );


  inlen = 1 << 24;
  mu_check ( validate_parameters (outlen, inlen, saltlen) == ERROR_INLEN_TOO_BIG);

  inlen = INLEN_MAX;
  mu_check ( validate_parameters (outlen, inlen, saltlen) == ERROR_INLEN_TOO_BIG);
}

void 
mu_test_validate_parameters__saltlen () 
{
  size_t outlen;
  size_t inlen;
  size_t saltlen;

  outlen = 32;
  inlen = 1024;
  saltlen = 32;
  mu_check ( validate_parameters (outlen, inlen, saltlen) == 0 );

  saltlen = 1 << 24;
  mu_check ( validate_parameters (outlen, inlen, saltlen) == ERROR_SALTLEN_TOO_BIG);

  saltlen = SALTLEN_MAX;
  mu_check ( validate_parameters (outlen, inlen, saltlen) == ERROR_SALTLEN_TOO_BIG);
}

void 
mu_test_options () 
{
  struct balloon_options opts;
  opts.s_cost = 0;
  opts.t_cost = 10000llu;
  opts.n_threads = 1;

  opts.s_cost = 1024;
  mu_check ( options_validate (&opts) == ERROR_NONE );

  opts.t_cost = 0;
  mu_check ( options_validate (&opts) != ERROR_NONE );

  opts.t_cost = 3;
  mu_check ( options_validate (&opts) == ERROR_NONE );
}

void 
mu_test_options__blocks () 
{
  struct balloon_options opts;
  opts.t_cost = 10000llu;
  opts.s_cost = 1024*1024;
  opts.n_threads = 1;

  mu_check ( !options_validate (&opts) );
}

