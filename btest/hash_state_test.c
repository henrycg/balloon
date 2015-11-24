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


#include <stdio.h>
#include "mutest.h"

#include <balloon.h>
#include "libballoon/constants.h"
#include "libballoon/errors.h"
#include "libballoon/hash_state.h"

static void
init_options (struct balloon_options *opts, enum mix_method mix)
{
  opts->m_cost = 1024 * 128;
  opts->t_cost = 3;
  opts->n_neighbors = 20;
  opts->comp_opts.comp = COMP__KECCAK_1600;
  opts->comp_opts.comb = COMB__HASH;
  opts->mix = mix;
  opts->n_threads = 1;
}

void 
test_hash_state_init_free (enum mix_method mix) 
{
  struct hash_state s;
  struct balloon_options opts;
 
  init_options (&opts, mix);
  mu_ensure ( !options_validate (&opts) );

  const unsigned char salt[] = "abcdefghijkl";
  mu_ensure ( !hash_state_init (&s, &opts, salt, sizeof (salt)) );
  hash_state_free (&s);
}

void 
mu_test_hash_state_init_free (void)
{
  for (int i = 0; i < MIX__END; i++)
    test_hash_state_init_free (i);
}

void 
test_hash_state_fill (enum mix_method mix) 
{
  struct hash_state s;
  struct balloon_options opts;
  init_options (&opts, mix);
  mu_ensure ( !options_validate (&opts) );
  const unsigned char salt[] = "abcdefghijkl";
  const unsigned char in[] = "ZZZZZZZZZZZZ";
  mu_ensure ( !hash_state_init (&s, &opts, salt, sizeof (salt)) );
  mu_ensure ( !hash_state_fill (&s, in, sizeof (in), salt, sizeof (salt)) ); 

  size_t blocks = s.n_blocks;
  if (mix == MIX__BALLOON_DOUBLE_BUFFER || mix == MIX__BALLOON_DOUBLE_BUFFER_PAR)
    blocks /= 2;
 
  if (mix != MIX__ARGON2_UNIFORM) {
    size_t n_zero = 0;
    for (size_t i = 0; i < s.block_size*blocks; i++) {
      if (!s.buffer[i]) n_zero++;
    }

    mu_check (n_zero < (blocks*s.block_size)/200);
  }

  hash_state_free (&s);
}

void 
mu_test_hash_state_fill (void)
{
  for (int i = 0; i < MIX__END; i++)
    test_hash_state_fill (i);
}

void 
test_hash_state_fill2 (enum mix_method mix) 
{
  struct balloon_options opts;
  init_options (&opts, mix);

  struct hash_state s1;
  struct hash_state s2;
  struct hash_state s3;
  unsigned char salt[] = "abcdefghijkl";
  unsigned char in[] = "ZZZZZZZZZZZZ";
  mu_ensure ( !hash_state_init (&s1, &opts, salt, sizeof (salt)) );
  mu_ensure ( !hash_state_fill (&s1, in, sizeof (in), salt, sizeof (salt)) ); 

  salt[0] = '5';
  mu_ensure ( !hash_state_init (&s2, &opts, salt, sizeof (salt)) );
  mu_ensure ( !hash_state_fill (&s2, in, sizeof (in), salt, sizeof (salt)) ); 


  in[3] = '7';
  mu_ensure ( !hash_state_init (&s3, &opts, salt, sizeof (salt)) );
  mu_ensure ( !hash_state_fill (&s3, in, sizeof (in), salt, sizeof (salt)) ); 

  if (mix != MIX__ARGON2_UNIFORM) {
    // These checks should all pass w.h.p.
    size_t collisions = 0;
    for (size_t i = 0; i < s1.n_blocks; i++) {
      if ( s1.buffer[i] == s2.buffer[i] ||
              s2.buffer[i] == s3.buffer[i] )
        collisions++;
    }

    mu_ensure (collisions < 50);
  }

  hash_state_free (&s1);
  hash_state_free (&s2);
  hash_state_free (&s3);
}

void 
mu_test_hash_state_fill2 (void)
{
  for (int i = 0; i < MIX__END; i++)
    test_hash_state_fill2 (i);
}

void 
test_hash_state_mix (enum mix_method mix) 
{
  struct balloon_options opts;
  init_options (&opts, mix);
  struct hash_state s;
  const unsigned char salt[] = "abcdefghijkl";
  const unsigned char in[] = "ZZZZZZZZZZZZ";
  mu_ensure ( !hash_state_init (&s, &opts, salt, sizeof (salt)) );
  mu_ensure ( !hash_state_fill (&s, in, sizeof (in), salt, sizeof (salt)) ); 

  for (int i = 0; i < 10; i++)
    mu_ensure ( !hash_state_mix (&s) );

  unsigned char out[1024];
  mu_ensure ( !hash_state_extract (&s, out, 1024) );
  mu_ensure ( out[0] || out[1] || out[2] || out[3] );
  hash_state_free (&s);
}

void 
mu_test_hash_state_mix (void)
{
  for (int i = 0; i < MIX__END; i++)
    test_hash_state_mix (i);
}

void 
test_hash_state_mix_threads (void)
{
  struct balloon_options opts;
  init_options (&opts, MIX__BALLOON_DOUBLE_BUFFER_PAR);
  opts.n_threads = 7;

  mu_ensure ( !options_validate (&opts) );
  struct hash_state s;
  const unsigned char salt[] = "abcdefghijkl";
  const unsigned char in[] = "ZZZZZZZZZZZZ";
  mu_ensure ( !hash_state_init (&s, &opts, salt, sizeof (salt)) );
  mu_ensure ( !hash_state_fill (&s, in, sizeof (in), salt, sizeof (salt)) ); 

  for (int i = 0; i < 10; i++)
    mu_ensure ( !hash_state_mix (&s) );

  unsigned char out[1024];
  mu_ensure ( !hash_state_extract (&s, out, 1024) );
  mu_ensure ( out[0] || out[1] || out[2] || out[3] );
  hash_state_free (&s);
}


void 
test_hash_state_bad_extract (enum mix_method mix) 
{
  struct balloon_options opts;
  init_options (&opts, mix);
  struct hash_state s;
  const unsigned char salt[] = "abcdefghijkl";
  const unsigned char in[] = "ZZZZZZZZZZZZ";
  mu_ensure ( !hash_state_init (&s, &opts, salt, sizeof (salt)) );
  mu_ensure ( !hash_state_fill (&s, in, sizeof (in), salt, sizeof (salt)) ); 

  unsigned char out[1024];
  mu_check ( hash_state_extract (&s, out, 1024) == ERROR_CANNOT_EXTRACT_BEFORE_MIX );
  hash_state_free (&s);
}

void 
mu_test_hash_state_bad_extract (void)
{
  for (int i = 0; i < MIX__END; i++)
    test_hash_state_bad_extract (i);
}


