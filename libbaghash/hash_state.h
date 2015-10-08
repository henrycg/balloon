#ifndef __HASH_STATE_H__
#define __HASH_STATE_H__

#include <stddef.h>

#include "bitstream.h"
#include "options.h"

struct hash_state;

typedef int func_hash_state_init (struct hash_state *s, struct baghash_options *opts, 
  const void *salt, size_t saltlen);
typedef int func_hash_state_free (struct hash_state *s);
typedef int func_hash_state_fill (struct hash_state *s, const void *in, size_t inlen,
    const void *salt, size_t saltlen);
typedef int func_hash_state_mix (struct hash_state *s);
typedef int func_hash_state_extract (struct hash_state *s, void *out, size_t outlen);

struct hash_state {
  size_t n_blocks;
  size_t block_size;
  unsigned char *buffer;
  struct bitstream bstream;
  struct baghash_options *opts;

  func_hash_state_init *f_init;
  func_hash_state_free *f_free;
  func_hash_state_fill *f_fill;
  func_hash_state_mix *f_mix;
  func_hash_state_extract *f_extract;

  void *extra_data;
};


int hash_state_init (struct hash_state *s, struct baghash_options *opts,
    const void *salt, size_t saltlen);

int hash_state_free (struct hash_state *s);

int hash_state_fill (struct hash_state *s, const void *in, size_t inlen,
    const void *salt, size_t saltlen);

int hash_state_mix (struct hash_state *s);

int hash_state_extract (struct hash_state *s, void *out, size_t outlen);


int fill_bytes_from_strings (struct hash_state *s, 
    unsigned char *block_start, size_t bytes_to_fill,
    const void *in, size_t inlen,
    const void *salt, size_t saltlen);

void *
block_index (const struct hash_state *s, size_t i);

void *
block_last (const struct hash_state *s);

#endif
