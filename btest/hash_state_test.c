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
init_options (struct balloon_options *opts)
{
  opts->s_cost = 128;
  opts->t_cost = 3;
  opts->n_threads = 1;
}

void 
mu_test_hash_state_init_free ()
{
  struct hash_state s;
  struct balloon_options opts;
 
  init_options (&opts);

  const unsigned char salt[] = "abcdefghijklabasdfasdfasdfasfcdefghijkl";
  mu_ensure ( !hash_state_init (&s, &opts, salt) );
  hash_state_free (&s);
}

void 
mu_test_hash_state_fill (void) 
{
  struct hash_state s;
  struct balloon_options opts;
  init_options (&opts);
  const unsigned char salt[] = "abcdefghijkabasdfasdfasdfasfcdefghijkll";
  const unsigned char in[] = "ZZZZZZZZZZZZ";
  mu_ensure ( !hash_state_init (&s, &opts, salt) );
  mu_ensure ( !hash_state_fill (&s, salt, in, sizeof (in)) ); 

  hash_state_free (&s);
}

void 
mu_test_hash_state_fill2 (void)
{
  struct balloon_options opts;
  init_options (&opts);

  struct hash_state s1;
  struct hash_state s2;
  struct hash_state s3;
  unsigned char salt[] = "abcdefghijklabasdfasdfasdfasfcdefghijkl";
  unsigned char in[] = "ZZZZZZZZZZZZ";
  mu_ensure ( !hash_state_init (&s1, &opts, salt) );
  mu_ensure ( !hash_state_fill (&s1, salt, in, sizeof (in)) ); 

  salt[0] = '5';
  mu_ensure ( !hash_state_init (&s2, &opts, salt) );
  mu_ensure ( !hash_state_fill (&s2, salt, in, sizeof (in)) ); 


  in[3] = '7';
  mu_ensure ( !hash_state_init (&s3, &opts, salt) );
  mu_ensure ( !hash_state_fill (&s3, salt, in, sizeof (in)) ); 

  hash_state_free (&s1);
  hash_state_free (&s2);
  hash_state_free (&s3);
}

void 
mu_test_hash_state_mix (void) 
{
  struct balloon_options opts;
  init_options (&opts);
  struct hash_state s;
  const unsigned char salt[] = "abcdefghijklabasdfasdfasdfasfcdefghijkl";
  const unsigned char in[] = "ZZZZZZZZZZZZ";
  mu_ensure ( !hash_state_init (&s, &opts, salt) );
  mu_ensure ( !hash_state_fill (&s, salt, in, sizeof (in)) ); 

  for (int i = 0; i < 10; i++)
    mu_ensure ( !hash_state_mix (&s) );

  unsigned char out[32];
  mu_ensure ( !hash_state_extract (&s, out) );
  mu_ensure ( out[0] || out[1] || out[2] || out[3] );
  hash_state_free (&s);
}


void 
mu_test_hash_state_bad_extract (void)
{
  struct balloon_options opts;
  init_options (&opts);
  struct hash_state s;
  const unsigned char salt[] = "abasdfasdfasdfasfcdefghijkl";
  const unsigned char in[] = "ZZZZZZZZZZZZ";
  mu_ensure ( !hash_state_init (&s, &opts, salt) );
  mu_ensure ( !hash_state_fill (&s, salt, in, sizeof (in)) ); 

  unsigned char out[32];
  mu_check ( hash_state_extract (&s, out) == ERROR_CANNOT_EXTRACT_BEFORE_MIX );
  hash_state_free (&s);
}

