
#include <baghash.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libbaghash/options.h"
#include "libbaghash/timing.h"

int main (int argc, char *argv[]){
  struct comp_options comp_opts;
  comp_opts.comp = 0;
  comp_opts.comb = 0;

  int xor_then_hash = false;
  int32_t n_rounds = 8;
  int64_t n_space = (1024*1024);
  int16_t n_neighbors = 0;
  int32_t n_iters = 1;
  enum mix_method mix = 0;

  while (1)
    {
      struct option long_options[] =
        {
          /* These options don’t set a flag.
             We distinguish them by their indices. */
          {"xor-then-hash", no_argument, &xor_then_hash, 1},
          {"comp",  required_argument, 0, 'c'},
          {"mix",  required_argument, 0, 'm'},
          {"space",  required_argument, 0, 's'},
          {"rounds",  required_argument, 0, 'r'},
          {"neighbors",  required_argument, 0, 'n'},
          {"iterations",  required_argument, 0, 'i'},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      char c = getopt_long (argc, argv, "xc:m:s:r:n:i:",
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
          else {
            fprintf (stderr, "Invalid compression method\n");
            return -1;
          }
          break;

        case 'm':
          if (!strcmp (optarg, "single"))
            mix = MIX__BAGHASH_SINGLE_BUFFER;
          else if (!strcmp (optarg, "double"))
            mix = MIX__BAGHASH_DOUBLE_BUFFER;
          else if (!strcmp (optarg, "argon2"))
            mix = MIX__ARGON2_UNIFORM;
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
          n_neighbors = strtoll (optarg, &end, 10);
          if (errno > 0 || *end != '\0' || n_neighbors < 0) {
            fprintf (stderr, "Invalid argument to -n\n");
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
          

        case '?':
          printf ("help\n");
          /* getopt_long already printed an error message. */
          break;

        default:
          return -1;
        }
    }

  if (optind + 2 < argc) {
      fprintf (stderr, "Too many arguments\n");
      return -1;
    }
  if (optind + 2 > argc)
    {
      fprintf (stderr, "Input and salt not passed in\n");
      return -1;
    }

  char *in = argv[optind];
  char *salt = argv[optind+1];


  comp_opts.comb = xor_then_hash ? COMB__XOR : COMB__HASH;
  struct baghash_options opts;
  opts.m_cost = n_space;
  opts.t_cost = n_rounds;
  opts.n_neighbors = n_neighbors;
  opts.comp_opts = comp_opts;
  opts.mix = mix;

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
  printf ("Mix            = %d\n", opts.mix);
  printf ("Compression    = %d\n", opts.comp_opts.comp);
  printf ("XOR-then-hash  = %d\n", opts.comp_opts.comb);
  printf ("Input          = %s\n", in);
  printf ("Salt           = %s\n", salt);

  unsigned char out[32];
  int error;
  const double wall_start = wall_sec ();
  for (int32_t i = 0; i < n_iters; i++) {
    if ((error = BagHash (out, 32, in, strlen (in), salt, strlen (salt), &opts))) {
      fprintf (stderr, "BagHash failed with error: %d\n", error);
      return -1;
    }
  }
  const double wall_end = wall_sec ();
  const double wall_diff = wall_end - wall_start;
  printf("Time total      : %lg\n", wall_diff);
  printf("Hashes per sec  : %lg\n", ((double) n_iters) / wall_diff);
  return 0;
}

