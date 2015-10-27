
#include <baghash.h>
#include <errno.h>
#include <papi.h>
#include <stdio.h>
#include <string.h>

#include "libbaghash/options.h"
#include "libbaghash/timing.h"

#define ITERS 32

static bool use_papi = false;

static void
run_once (struct baghash_options *opts)
{
  const int n_events = 2;
  int events[n_events];
  events[0] = PAPI_L1_DCM;
  events[1] = PAPI_L2_DCM;
  //events[2] = PAPI_BR_PRC;
  
  int error;
  if (use_papi) {
    if ((error = PAPI_start_counters (events, n_events)) != PAPI_OK) {
      const char *e = PAPI_strerror (error);
      fprintf (stderr, "PAPI start failed with error: %s\n", e ? e : "");
      return;
    }
  }

  const char in[] = "test input";
  const char salt[] = "test salt";

  const double wall_start = wall_sec ();
  long long counters[n_events];
  for (int i = 0; i < n_events; i++) {
    counters[i] = 0;
  }
  const clock_t clk_start = rdtsc ();

  unsigned char out[32];
  for (int i = 0; i < ITERS; i++) {
    if ((error = BagHash (out, 32, in, strlen (in), salt, strlen (salt), opts))) {
      fprintf (stderr, "BagHash failed with error: %d\n", error);
      return;
    }
  }

  const clock_t clk_end = rdtsc ();
  const double wall_end = wall_sec ();
  const unsigned int bytes_total = opts->m_cost * opts->t_cost * ITERS;
  const unsigned int clks_total = (clk_end - clk_start);
  const double cpb = (double)clks_total/(double)bytes_total;
  const double wall_total = (wall_end - wall_start)/((double)ITERS);

  if (use_papi) {
    if ((error = PAPI_stop_counters (counters, n_events)) != PAPI_OK) {
      const char *e = PAPI_strerror (error);
      fprintf (stderr, "PAPI start failed with error: %s\n", e ? e : "");
      return;
    }
  }

  printf ("%d\t%d\t%d\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu8 "\t%lg\t%u\t%u\t%lg\t%lld\t%lld\n", 
      opts->mix,
      opts->comp_opts.comp,
      opts->comp_opts.comb,
      opts->m_cost,
      opts->t_cost,
      opts->n_neighbors,
      wall_total,
      bytes_total, 
      clks_total,
      cpb,
      counters[0]/ITERS,
      counters[1]/ITERS);
}

static void
bench_neighbors (void)
{
  // Run number of neighbors
  struct comp_options comp_opts;
  comp_opts.comp = COMP__KECCAK_1600;
  comp_opts.comb = COMB__XOR;

  struct baghash_options opts;
  opts.m_cost = 128 * 1024; 
  opts.t_cost = 5;
  opts.comp_opts = comp_opts;
  opts.mix = MIX__BAGHASH_DOUBLE_BUFFER;

  for (unsigned n_neighb = 1; n_neighb < 128; n_neighb += 2) {
    opts.n_neighbors = n_neighb;
    run_once (&opts); 
  }
}

static void
bench_mix (void)
{
  struct comp_options comp_opts;
  comp_opts.comp = COMP__KECCAK_1600;

  struct baghash_options opts;
  opts.t_cost = 5;
  opts.comp_opts = comp_opts;
  for (unsigned m_cost = 4*1024; m_cost < 4*1024*1024 + 1; m_cost *= 2) {
    for (int mix = 0; mix < MIX__END; mix++) {
      for (int comb = 0; comb < COMB__END; comb ++) {
        opts.comp_opts.comb = comb;
        opts.mix = mix;
        opts.m_cost = m_cost;
        opts.n_neighbors = options_n_neighbors (&opts);

        // Skip invalid combinations
        if (options_validate (&opts))
          continue;

        run_once (&opts); 
      }
    }
  }
}

static void
bench_hash (void)
{
  struct comp_options comp_opts;
  comp_opts.comb = COMB__XOR;

  struct baghash_options opts;
  opts.m_cost = 128 * 1024;
  opts.t_cost = 5;
  opts.comp_opts = comp_opts;
  opts.mix = MIX__BAGHASH_DOUBLE_BUFFER;

  for (unsigned m_cost = 4*1024; m_cost < 4*1024*1024 + 1; m_cost *= 2) {
    for (int comp = 0; comp < COMP__END; comp++) {
      opts.m_cost = m_cost;
      opts.comp_opts.comp = comp;
      opts.n_neighbors = options_n_neighbors (&opts);

      // Skip invalid combinations
      if (options_validate (&opts))
        continue;

      run_once (&opts); 
    }
  }
}


int
main (int argc, char *argv[])
{
  if (argc < 2 || (argc == 3 && strcmp (argv[2], "-p"))) {
    fprintf (stderr, "Usage: %s [test name] -p\n", argv[0]);
    return -1;
  }

  use_papi = argc > 2;

  printf ("Mix\tComp\tComb\tMCost\tTCost\tNeighb\tWall\tBytesTotal\tCycles\tCpb\tL1miss\tL2miss\n");

  if (!strcmp (argv[1], "neighbors")) {
    bench_neighbors (); 
  } else if (!strcmp (argv[1], "mix")) {
    bench_mix ();
  } else if (!strcmp (argv[1], "hash")) {
    bench_hash ();
  } else {
    fprintf (stderr, "Unknown benchmark\n");
    return 1;
  }

  return 0;
}

