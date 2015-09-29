#ifndef __HASH_STATE_H__
#define __HASH_STATE_H__

#include <stddef.h>

#include "bitstream.h"
#include "options.h"

struct hash_state {
  size_t n_blocks;
  size_t block_size;
  unsigned char *buffer;
  struct bitstream bstream;
  struct baghash_options *opts;
};

int hash_state_init (struct hash_state *s, struct baghash_options *opts,
    const void *salt, size_t saltlen);

int hash_state_free (struct hash_state *s);

int hash_state_fill (struct hash_state *s, const void *in, size_t inlen,
    const void *salt, size_t saltlen);

int hash_state_mix (struct hash_state *s);

int hash_state_extract (struct hash_state *s, void *out, size_t outlen);

#endif
