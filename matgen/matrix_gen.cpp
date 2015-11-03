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


#include <random>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "libbaghash/errors.h"
#include "secure_random.h"

struct matrix_generator {
  secure_random *rng;
  std::binomial_distribution<> *binom;
};

extern "C" 
struct matrix_generator * 
matrix_generator_init (struct bitstream *b, size_t n_rows)
{
  struct matrix_generator *m = (struct matrix_generator *)malloc (sizeof (*m));
  if (!m) return NULL;

  // TODO: Make sure that the bias of this generator
  // doesn't invalidate the security proofs.

  /*
  const double alpha = 4.0f;
  const double dn_rows = n_rows;
  const double n_over_alpha_plus_one = (dn_rows/alpha) + 1;
  const double p = (alpha / dn_rows) * log (n_over_alpha_plus_one); 
  */
  const double p = (n_rows <= 20) ? 1.0f : 20.0f / ((double) n_rows);
  m->binom = new (std::nothrow) std::binomial_distribution<>(n_rows, p);
  if (!m->binom)
    return NULL;
 
  m->rng = new (std::nothrow) secure_random(b);
  if (!m->rng) {
    delete m->binom;
    return NULL;
  }

  return m;
}

extern "C" void
matrix_generator_free (struct matrix_generator *m)
{
  delete m->rng;
  delete m->binom;
  free (m);
}

extern "C" int 
matrix_generator_row_weight (struct matrix_generator *m, size_t *out)
{
  *out = (*m->binom)(*m->rng);
  return ERROR_NONE;
}

