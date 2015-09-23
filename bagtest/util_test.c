
#include "mutest.h"

#include "libbaghash/constants.h"
#include "libbaghash/errors.h"
#include "libbaghash/util.h"

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

