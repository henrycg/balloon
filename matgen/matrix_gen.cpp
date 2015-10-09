
#include <boost/math/distributions/binomial.hpp>
#include <boost/random.hpp>
#include <boost/random/variate_generator.hpp>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "libbaghash/errors.h"
#include "secure_random.h"

using boost::random::binomial_distribution;
using boost::random::variate_generator;

struct matrix_generator {
  secure_random *rng;
  binomial_distribution<> *binom;
  variate_generator<secure_random&, binomial_distribution<> > *gen;
};

extern "C" 
struct matrix_generator * 
matrix_generator_init (struct bitstream *b, size_t n_rows, int c)
{
  struct matrix_generator *m = (struct matrix_generator *)malloc (sizeof (*m));
  if (!m) return NULL;

  double p = (2 * (log (n_rows) + (double)c - 1.0f)) / ((double)n_rows);
  m->binom = new (std::nothrow) binomial_distribution<> (n_rows, p);
  if (!m->binom)
    return NULL;
 
  m->rng = new (std::nothrow) secure_random(b);
  if (!m->rng) {
    delete m->binom;
    return NULL;
  }

  // TODO: Make sure that the bias of this generator
  // doesn't invalidate the security proofs.
  m->gen = new (std::nothrow) variate_generator<secure_random&, binomial_distribution<> > (*m->rng, *m->binom);
  if (!m->gen) {
    delete m->binom;
    delete m->rng;
    return NULL;
  }

  return m;
}

extern "C" void
matrix_generator_free (struct matrix_generator *m)
{
  delete m->gen;
  delete m->rng;
  delete m->binom;
  free (m);
}

extern "C" int 
matrix_generator_row_weight (struct matrix_generator *m, size_t *out)
{
  *out = (*m->gen)();
  return ERROR_NONE;
}

