
#include <stddef.h>

#include "compress.h"
#include "errors.h"

int compress (unsigned char *out, unsigned const char *blockA, unsigned const char *blockB,
    size_t block_size)
{
  // TODO: put in real compression function
  for (size_t i = 0; i < block_size; i++) {
    *(out + i) = blockA[i] ^ blockB[i];
  }

  return ERROR_NONE;
}

