
#include <baghash.h>
#include <string.h>

#include "mutest.h"

#include "libbaghash/compress.h"
#include "libbaghash/options.h"


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
      mu_ensure (!compress (out, blocks, 2, &comp));
    } 
  }
}

