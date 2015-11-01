
#include <baghash.h>

#include "mutest.h"

#include "libbaghash/baghash.c"
#include "libbaghash/constants.h"
#include "libbaghash/errors.h"
#include "libbaghash/options.h"

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

  inlen = 0;
  mu_check ( validate_parameters (outlen, inlen, saltlen) == ERROR_INLEN_TOO_SMALL );

  inlen = INLEN_MIN - 1;
  mu_check ( validate_parameters (outlen, inlen, saltlen) == ERROR_INLEN_TOO_SMALL );

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

  saltlen = 0;
  mu_check ( validate_parameters (outlen, inlen, saltlen) == ERROR_SALTLEN_TOO_SMALL );

  saltlen = SALTLEN_MIN - 1;
  mu_check ( validate_parameters (outlen, inlen, saltlen) == ERROR_SALTLEN_TOO_SMALL );

  saltlen = 1 << 24;
  mu_check ( validate_parameters (outlen, inlen, saltlen) == ERROR_SALTLEN_TOO_BIG);

  saltlen = SALTLEN_MAX;
  mu_check ( validate_parameters (outlen, inlen, saltlen) == ERROR_SALTLEN_TOO_BIG);
}

void 
mu_test_options () 
{
  struct baghash_options opts;
  opts.m_cost = 0;
  opts.t_cost = 10000llu;
  opts.n_neighbors = 1;
  opts.comp_opts.comp = 0;
  opts.comp_opts.comb = 0;
  opts.mix = 0; 
  opts.n_threads = 1;

  opts.m_cost = 1024;
  mu_check ( !options_validate (&opts) );

  opts.t_cost = 0;
  mu_check ( !options_validate (&opts) );

  opts.t_cost = 3;
  opts.n_neighbors = 0;
  mu_check ( options_validate (&opts) );
}

void 
mu_test_options__blocks () 
{
  struct baghash_options opts;
  opts.t_cost = 10000llu;
  opts.m_cost = 1024*1024;
  opts.n_neighbors = 1;
  opts.comp_opts.comp = 0;
  opts.comp_opts.comb = 0;
  opts.mix = 0;
  opts.n_threads = 1;

  mu_check ( !options_validate (&opts) );
}
