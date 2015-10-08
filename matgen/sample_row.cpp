
#include <boost/math/distributions/binomial.hpp>
#include <boost/random.hpp>
#include <boost/random/variate_generator.hpp>
#include <math.h>

#include "libbaghash/errors.h"
#include "secure_random.h"

using boost::random::binomial_distribution;
using boost::random::variate_generator;

extern "C" int 
nonzero_elms_in_row (struct bitstream *b, size_t *row_index, size_t n_rows, int c);

extern "C" int 
nonzero_elms_in_row (struct bitstream *b, size_t *row_index, size_t n_rows, int c)
{
  long double p = log (n_rows) + c;
  binomial_distribution<> binom (n_rows, p);
  secure_random rng (b);

  variate_generator<secure_random&, binomial_distribution<> > coins(rng, binom);
  *row_index = coins ();
  return ERROR_NONE;
}

