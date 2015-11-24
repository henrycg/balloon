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


#include <balloon.h>
#include <errno.h>
#ifdef HAS_PAPI_H
#include <papi.h>
#endif
#include <stdio.h>
#include <string.h>

#include "libballoon/options.h"
#include "libballoon/timing.h"

#define ITERS 32
#define MEM_MAX (32 * 1024 * 1024)

static bool use_papi = false;

static void
run_once (struct balloon_options *opts)
{
  const int n_events = 2;
#ifdef HAS_PAPI_H
  int events[n_events];
  events[0] = PAPI_L1_DCM;
  events[1] = PAPI_L2_DCM;
  //events[2] = PAPI_BR_PRC;
#endif
  
  int error;
  if (use_papi) {
#ifdef HAS_PAPI_H
    if ((error = PAPI_start_counters (events, n_events)) != PAPI_OK) {
      const char *e = PAPI_strerror (error);
      fprintf (stderr, "PAPI start failed with error: %s\n", e ? e : "");
      return;
    }
#else 
    fprintf (stderr, "Not compiled with PAPI support.\n");
    return;
#endif
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
    if ((error = BalloonHash (out, 32, in, strlen (in), salt, strlen (salt), opts))) {
      fprintf (stderr, "BalloonHash failed with error: %d\n", error);
      return;
    }
  }

  const clock_t clk_end = rdtsc ();
  const double wall_end = wall_sec ();
  const unsigned int bytes_total = opts->m_cost * opts->t_cost * ITERS;
  const unsigned int clks_total = (clk_end - clk_start);
  const double cpb = (double)clks_total/(double)bytes_total;
  const double wall_total = (wall_end - wall_start)/((double)ITERS);

#ifdef HAS_PAPI_H
  if (use_papi) {
    if ((error = PAPI_stop_counters (counters, n_events)) != PAPI_OK) {
      const char *e = PAPI_strerror (error);
      fprintf (stderr, "PAPI start failed with error: %s\n", e ? e : "");
      return;
    }
  }
#endif

  printf ("%d\t", opts->mix);
  printf ("%d\t", opts->comp_opts.comp);
  printf ("%d\t", opts->comp_opts.comb);
  printf ("%" PRIu64 "\t", opts->m_cost);
  printf ("%" PRIu64 "\t", opts->t_cost);
  printf ("%" PRIu8 "\t", opts->n_neighbors);
  printf ("%" PRIu16 "\t", opts->n_threads);
  printf ("%lg\t", wall_total);
  printf ("%u\t", bytes_total);
  printf ("%u\t", clks_total);
  printf ("%lg\t", cpb);
  printf ("%lld\t", counters[0]/ITERS);
  printf ("%lld\t", counters[1]/ITERS);
  printf ("\n");
}

static void
bench_neighbors (void)
{
  // Run number of neighbors
  struct comp_options comp_opts;
  comp_opts.comp = COMP__KECCAK_1600;

  struct balloon_options opts;
  opts.m_cost = 128 * 1024; 
  opts.t_cost = 5;
  opts.n_threads = 1;
  opts.comp_opts = comp_opts;
  opts.mix = MIX__BALLOON_DOUBLE_BUFFER;

  for (int comb = 0; comb < COMB__END; comb++) {
    comp_opts.comb = comb;
    opts.comp_opts = comp_opts;
    for (unsigned n_neighb = 1; n_neighb < 128; n_neighb += 2) {
      opts.n_neighbors = n_neighb;
      run_once (&opts); 
    }
  }
}

static void
bench_mix (void)
{
  struct comp_options comp_opts;
  comp_opts.comp = COMP__KECCAK_1600;

  struct balloon_options opts;
  opts.t_cost = 5;
  opts.comp_opts = comp_opts;
  opts.n_threads = 1;
  for (unsigned m_cost = 4*1024; m_cost < MEM_MAX + 1; m_cost *= 2) {
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
bench_threads (void)
{
  struct comp_options comp_opts;
  comp_opts.comp = COMP__KECCAK_1600;

  struct balloon_options opts;
  opts.t_cost = 3;
  opts.comp_opts = comp_opts;
  opts.m_cost = 1*1024*1024;
  opts.mix = MIX__BALLOON_DOUBLE_BUFFER_PAR;
  for (unsigned threads = 1; threads < 9; threads++) {
    for (int comb = 0; comb < COMB__END; comb ++) {
      opts.comp_opts.comb = comb;
      opts.n_threads = threads;
      opts.n_neighbors = options_n_neighbors (&opts);

      // Skip invalid combinations
      if (options_validate (&opts))
        continue;

      run_once (&opts); 
    }
  }
}
static void
bench_hash (void)
{
  struct comp_options comp_opts;
  comp_opts.comb = COMB__XOR;

  struct balloon_options opts;
  opts.m_cost = 128 * 1024;
  opts.t_cost = 5;
  opts.comp_opts = comp_opts;
  opts.mix = MIX__BALLOON_DOUBLE_BUFFER;
  opts.n_threads = 1;

  for (unsigned m_cost = 4*1024; m_cost < MEM_MAX + 1; m_cost *= 2) {
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

  printf ("Mix\tComp\tComb\tMCost\tTCost\tNeighb\tThreads\tWall\tBytesTotal\tCycles\tCpb\tL1miss\tL2miss\n");

  if (!strcmp (argv[1], "neighbors")) {
    bench_neighbors (); 
  } else if (!strcmp (argv[1], "mix")) {
    bench_mix ();
  } else if (!strcmp (argv[1], "hash")) {
    bench_hash ();
  } else if (!strcmp (argv[1], "threads")) {
    bench_threads ();
  } else {
    fprintf (stderr, "Unknown benchmark\n");
    return 1;
  }

  return 0;
}

