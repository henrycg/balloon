
#include <baghash.h>

#include "errors.h"

#define OUTLEN_MIN 16
#define OUTLEN_MAX 128

#define INLEN_MIN 4
#define INLEN_MAX (1<<20)

#define SALTLEN_MIN 4
#define SALTLEN_MAX (1<<20)

int
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


