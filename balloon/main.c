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
#include <getopt.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libballoon/options.h"
#include "libballoon/timing.h"

static void 
usage (const char *name)
{
  fprintf (stderr, "Usage: %s password salt\n", name);
  fprintf (stderr, "A test utility for the Balloon family of hash functions.\n\n");
  fprintf (stderr, "  -c, --comp=TYPE       Compression function to use. Options are:\n");
  fprintf (stderr, "                            Default = keccak\n");
  fprintf (stderr, "                            argon    -- Argon2's version of Blake2b\n");
  fprintf (stderr, "                            blake2b  -- standard Blake2b\n");
  fprintf (stderr, "                            echo     -- ECHO 512-bit double-pipe\n");
  fprintf (stderr, "                            keccak   -- SHA-3\n");
  fprintf (stderr, "                            sempira2048 -- Sempira (2048 bytes)\n");
  fprintf (stderr, "                            sha512\n");
  fprintf (stderr, "  -h, --help            Print this help message.\n");
  fprintf (stderr, "  -i, --iterations=NUM  Number of hashes to compute (for perf testing).\n");
  fprintf (stderr, "                            Default = 1\n");
  fprintf (stderr, "  -m, --mix=TYPE        Mixing method. Options are:\n");
  fprintf (stderr, "                            Default = single\n");
  fprintf (stderr, "                            single      -- Single buffer\n");
  fprintf (stderr, "                            double      -- Double buffer\n");
  fprintf (stderr, "                            double-par  -- with parallelism\n");
  fprintf (stderr, "                            double-pipe -- with pipelining\n");
  fprintf (stderr, "                            argon2      -- Argon2i-style mixing\n");
  fprintf (stderr, "                            catena-brg  -- Catena Bit-Reversal Graph mixing\n");
  fprintf (stderr, "                            scrypt      -- Scrypt-style mixing\n");
  fprintf (stderr, "  -n, --neighbors=NUM   Number of neighboring block hashed at each step.\n");
  fprintf (stderr, "                            Default = [depends on parameter choices]\n");
  fprintf (stderr, "  -r, --rounds=NUM      Number of mixing rounds.\n");
  fprintf (stderr, "                            Default = 8\n");
  fprintf (stderr, "  -s, --space=NUM       Space usage (in bytes).\n");
  fprintf (stderr, "                            Default = 1024 KB\n");
  fprintf (stderr, "  -t, --threads=NUM     Number of threads to use.\n");
  fprintf (stderr, "                            Default = 1\n");
  fprintf (stderr, "  -x, --xor             Use linear/XOR-based construction.\n");
  fprintf (stderr, "                            Default = 0\n");
}

int 
main (int argc, char *argv[]) 
{
  struct comp_options comp_opts = {
    .comp = COMP__BLAKE_2B,
    .comb = COMB__HASH
  };

  int xor_then_hash = false;
  int32_t n_rounds = 8;
  int64_t n_space = (1024*1024);
  int16_t n_neighbors = 0;
  int32_t n_iters = 1;
  int16_t n_threads = 1;
  int help = false;
  enum mix_method mix = 0;

  while (1)
    {
      struct option long_options[] =
        {
          /* These options don’t set a flag.
             We distinguish them by their indices. */
          {"xor", no_argument, &xor_then_hash, 1},
          {"comp",  required_argument, 0, 'c'},
          {"mix",  required_argument, 0, 'm'},
          {"space",  required_argument, 0, 's'},
          {"rounds",  required_argument, 0, 'r'},
          {"neighbors",  required_argument, 0, 'n'},
          {"iterations",  required_argument, 0, 'i'},
          {"threads",  required_argument, 0, 't'},
          {"help", no_argument, &help, 1},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      char c = getopt_long (argc, argv, "xc:m:s:r:n:i:t:h?",
                       long_options, &option_index);
      char *end;

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
        {
        case 0:
          /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0)
            break;
            printf ("option %s", long_options[option_index].name);
            if (optarg)
              printf (" with arg %s", optarg);
          printf ("\n");
          break;

        case 'x':
          xor_then_hash = true;
          break; 
        case 'c':
          if (!strcmp (optarg, "keccak"))
            comp_opts.comp = COMP__KECCAK_1600;
          else if (!strcmp (optarg, "argon"))
            comp_opts.comp = COMP__ARGON;
          else if (!strcmp (optarg, "blake2b"))
            comp_opts.comp = COMP__BLAKE_2B;
          else if (!strcmp (optarg, "sha512"))
            comp_opts.comp = COMP__SHA_512;
          else if (!strcmp (optarg, "sempira2048"))
            comp_opts.comp = COMP__SEMPIRA_2048;
          else if (!strcmp (optarg, "echo"))
            comp_opts.comp = COMP__ECHO;
          else {
            fprintf (stderr, "Invalid compression method\n");
            return -1;
          }
          break;

        case 'm':
          if (!strcmp (optarg, "single"))
            mix = MIX__BALLOON_SINGLE_BUFFER;
          else if (!strcmp (optarg, "double"))
            mix = MIX__BALLOON_DOUBLE_BUFFER;
          else if (!strcmp (optarg, "double-par"))
            mix = MIX__BALLOON_DOUBLE_BUFFER_PAR;
          else if (!strcmp (optarg, "double-pipe"))
            mix = MIX__BALLOON_DOUBLE_BUFFER_PIPE;
          else if (!strcmp (optarg, "argon2"))
            mix = MIX__ARGON2_UNIFORM;
          else if (!strcmp (optarg, "catena-brg"))
            mix = MIX__CATENA_BRG;
          else if (!strcmp (optarg, "catena-dbg"))
            mix = MIX__CATENA_DBG;
          else if (!strcmp (optarg, "scrypt"))
            mix = MIX__SCRYPT;
          else {
            fprintf (stderr, "Invalid mix method\n");
            return -1;
          }
          break;

        case 's':
          errno = 0;
          n_space = strtoll (optarg, &end, 10);
          if (errno > 0 || *end != '\0' || n_space < 0) {
            fprintf (stderr, "Invalid argument to -s\n");
            return -1;
          }

          break;

        case 'n':
          errno = 0;
          n_neighbors = strtoll (optarg, &end, 4);
          if (errno > 0 || *end != '\0' || n_neighbors < 0) {
            fprintf (stderr, "Invalid argument to -n\n");
            return -1;
          }
          break;

        case 't':
          errno = 0;
          n_threads = strtoll (optarg, &end, 10);
          if (errno > 0 || *end != '\0' || n_threads <= 0) {
            fprintf (stderr, "Invalid argument to -t\n");
            return -1;
          }
          break;

        case 'r':
          errno = 0;
          n_rounds = strtoll (optarg, &end, 10);
          if (errno > 0 || *end != '\0' || n_rounds < 0) {
            fprintf (stderr, "Invalid argument to -r\n");
            return -1;
          }
          break;

        case 'i':
          errno = 0;
          n_iters = strtoll (optarg, &end, 10);
          if (errno > 0 || *end != '\0' || n_iters < 0) {
            fprintf (stderr, "Invalid argument to -i\n");
            return -1;
          }
          break;
          

        case 'h':
        case '?':
          help = true;
          break;

        default:
          return -1;
        }
    }

  if (help) {
    usage (argv[0]);
    return 0;
  } else {
    if (optind + 2 < argc) {
        fprintf (stderr, "Too many arguments\n");
        return -1;
      }
    if (optind + 2 > argc)
      {
        fprintf (stderr, "Input and salt not passed in\n");
        return -1;
      }
  }

  char *in = argv[optind];
  char *salt = argv[optind+1];


  comp_opts.comb = xor_then_hash ? COMB__XOR : COMB__HASH;
  struct balloon_options opts = {
    .m_cost = n_space,
    .t_cost = n_rounds,
    .n_neighbors = n_neighbors,
    .n_threads = n_threads,
    .comp_opts = comp_opts,
    .mix = mix
  };

  const unsigned int rec_neighbs = options_n_neighbors (&opts);
  if (n_neighbors && ((uint16_t) n_neighbors) != rec_neighbs) {
    fprintf (stderr, "Warning: using unrecommended n_neighbors param!\n");
  }
  if (!n_neighbors)
    opts.n_neighbors = rec_neighbs;

  printf ("NRounds        = %lld\n", (long long int)opts.t_cost);
  printf ("NSpace         = %lld\n", (long long int)opts.m_cost);
  printf ("Neighbs        = %lld\n", (long long int)opts.n_neighbors);
  printf ("Niters         = %lld\n", (long long int)n_iters);
  printf ("Nthreads       = %d\n", (int)n_threads);
  printf ("Mix            = %d\n", opts.mix);
  printf ("Compression    = %d\n", opts.comp_opts.comp);
  printf ("XOR-then-hash  = %d\n", opts.comp_opts.comb);
  printf ("Input          = %s\n", in);
  printf ("Salt           = %s\n", salt);

  const int outlen = 32;
  unsigned char out[outlen];
  int error;
  const double wall_start = wall_sec ();
  for (int32_t i = 0; i < n_iters; i++) {
    if ((error = BalloonHash (out, outlen, in, strlen (in), salt, strlen (salt), &opts))) {
      fprintf (stderr, "BalloonHash failed with error: %d\n", error);
      return -1;
    }
  }
  const double wall_end = wall_sec ();
  const double wall_diff = wall_end - wall_start;
  printf("Time total      : %lg\n", wall_diff);
  printf("Hashes per sec  : %lg\n", ((double) n_iters) / wall_diff);
  printf("Output          : ");
  for (int i = 0; i < outlen; i++) {
    printf("%x", out[i]);
  }
  printf("\n");

  // Clean up OpenSSL junk
  EVP_cleanup();
  CRYPTO_cleanup_all_ex_data();
  ERR_remove_state(0);
  ERR_free_strings();
  return 0;
}

