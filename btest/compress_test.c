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
#include <string.h>

#include "mutest.h"

#include "libballoon/compress.h"
#include "libballoon/options.h"


void 
mu_test_compress () 
{
  for (int j=0; j<COMB__END; j++) {
    for (int i=0; i<COMP__END; i++) {
      struct comp_options comp;
      comp.comp = i;
      comp.comb = j;
      size_t block_size = compress_block_size (i);
      unsigned char out[block_size];
      unsigned char bufA[block_size];
      unsigned char bufB[block_size];
      memset (out, 0, block_size);

      unsigned const char *blocks[2];
      blocks[0] = bufA;
      blocks[1] = bufB;
      if (i == COMP__SIMPIRA_2048) { 
          if (j == COMB__HASH) 
            mu_ensure (!compress (out, blocks, 1, &comp));
      } else
        mu_ensure (!compress (out, blocks, 2, &comp));
    } 
  }
}

