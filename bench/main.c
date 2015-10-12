
#include <baghash.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "libbaghash/options.h"

uint64_t rdtsc (void)
{
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

double wall_sec (void)
{
    struct timeval t;
    gettimeofday (&t, NULL);
    return (double)t.tv_sec + ((double)t.tv_usec/1000000.0);
}

static void
run_once (struct baghash_options *opts)
{
  const char in[] = "test input";
  const char salt[] = "test salt";

  const clock_t clk_start = rdtsc ();
  const double wall_start = wall_sec ();

  unsigned char out[32];
  int error;
  if ((error = BagHash (out, 32, in, strlen (in), salt, strlen (salt), opts))) {
    fprintf (stderr, "BagHash failed with error: %d\n", error);
    return;
  }

  const clock_t clk_end = rdtsc ();
  const double wall_end = wall_sec ();
  const unsigned int bytes_total = opts->m_cost * opts->t_cost;
  const unsigned int clks_total = (unsigned int)(clk_end - clk_start);
  const double cpb = (double)clks_total/(double)bytes_total;
  const double wall_total = wall_end - wall_start;

  printf ("%d\t%d\t%d\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu8 "\t%lg\t%u\t%u\t%lg\n", 
      opts->mix,
      opts->comp_opts.comp,
      opts->comp_opts.comb,
      opts->m_cost,
      opts->t_cost,
      opts->n_neighbors,
      wall_total,
      bytes_total, 
      clks_total,
      cpb);
}

int
main (void)
{
  struct comp_options comp_opts;
  comp_opts.comp = 0;
  comp_opts.comb = 0;

  printf ("Mix\tComp\tComb\tMCost\tTCost\tNeighb\tWall\tBytesTotal\tCycles\tCpb\n");
  struct baghash_options opts;
  opts.m_cost = 0;
  opts.t_cost = 5;
  opts.comp_opts = comp_opts;
  opts.mix = 0;

  for (unsigned m_cost = 256*1024; m_cost < 32*1024*1024 + 1; m_cost *= 2) {
    opts.m_cost = m_cost;
    for (int i = 0; i < MIX__END; i++) {
      opts.mix = i;
      for (int j = 0; j < COMP__END; j++) {
        opts.comp_opts.comp = j;
        for (int k = 0; k < COMB__END; k++) {
          opts.comp_opts.comb = k;
          opts.n_neighbors = options_n_neighbors (&opts);

          // Skip invalid combinations
          if (options_validate (&opts))
            continue;

          run_once (&opts);
        }
      }
    }
  }

  return 0;
}

