#ifndef __BAGHASH_H__
#define __BAGHASH_H__

#include <stdbool.h>
#include <stddef.h>

enum comp_method {
  COMP__KECCAK_1600,
  COMP__ARGON_BLAKE2B,
  COMP__SHA_512,

  COMP__END
};

enum comb_method {
  COMB__HASH,
  COMB__XOR,

  COMB__END
};

struct comp_options {
  enum comp_method comp;
  enum comb_method comb;
};

// TODO: Add parallel two-buffer mixing method
enum mix_method {
  MIX__BAGHASH_SINGLE_BUFFER,
  MIX__BAGHASH_DOUBLE_BUFFER,

  MIX__ARGON2_UNIFORM,

  MIX__END
};

struct baghash_options {
  unsigned int m_cost;
  unsigned int t_cost;

  // Degree of expander graph used in the construction.
  // TODO: Make sure that this number is big enough to 
  //       get the expansion we need.
  // TODO: Set n_neighbors automatically and give warning
  //       when user overrides the default.
  unsigned int n_neighbors;
  struct comp_options comp_opts;
  enum mix_method mix;
};

int BagHash (void *out, size_t outlen, const void *in, size_t inlen, 
    const void *salt, size_t saltlen, 
    struct baghash_options *opts);

#endif /* __BAGHASH_H__ */

