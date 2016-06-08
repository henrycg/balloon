/*
 * Copyright (c) 2015-6, Henry Corrigan-Gibbs
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
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "libballoon/compress.h"
#include "libballoon/errors.h"


void 
print_image (struct comp_options *opts, uint32_t width, uint32_t height)
{
  const uint16_t block_size = compress_block_size (opts->comp);
  uint8_t data[block_size];
  uint8_t out[block_size];
  const uint8_t *bptr[1] = { data };
  int error;
  for (uint32_t w = 0; w < width; w++) {
    memset (out, 0, block_size);
    memset (data, 0, block_size);
  
    data[0] = w % 0xff;
    data[1] = (w & (~0xff)) >> 8;

    error = compress (out, bptr, 1, opts);
    if (error != ERROR_NONE) {
      fprintf (stderr, "Compression failed with error %d\n", error);
      return;
    }

    for (uint32_t h = 0; h < height && h < block_size; h++) {
      putchar (out[h]);
      putchar ((out[h]*out[h])%256);
      putchar ((out[h]*out[h]*out[h])%256);
    }
  }
}


int 
main (int argc, char *argv[]) 
{
  if (argc != 4) {
    fprintf (stderr, "Usage: %s comp_function width height\n", argv[0]);
    return -1;
  }

  struct comp_options comp_opts = {
    .comp = 0,
    .comb = COMB__HASH
  };

  if (!strcmp (argv[1], "keccak"))
    comp_opts.comp = COMP__KECCAK_1600;
  else if (!strcmp (argv[1], "argon"))
    comp_opts.comp = COMP__ARGON;
  else if (!strcmp (argv[1], "blake2b"))
    comp_opts.comp = COMP__BLAKE_2B;
  else if (!strcmp (argv[1], "sha512"))
    comp_opts.comp = COMP__SHA_512;
  else if (!strcmp (argv[1], "echo"))
    comp_opts.comp = COMP__ECHO;
  else if (!strcmp (argv[1], "simpira2048"))
    comp_opts.comp = COMP__SIMPIRA_2048;
  else {
    fprintf (stderr, "Invalid compression method\n");
    return -1;
  }
 
  errno = 0; 
  const int32_t width = strtol (argv[2], NULL, 10);
  if (errno || width <= 0) {
    fprintf (stderr, "Width must be a positive integer\n");
    return -1;
  }

  errno = 0; 
  const int32_t height = strtol (argv[3], NULL, 10);
  if (errno || height <= 0) {
    fprintf (stderr, "Height must be a positive integer\n");
    return -1;
  }

  print_image (&comp_opts, width, height);

  return 0;
}

