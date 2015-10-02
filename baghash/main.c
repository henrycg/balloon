
#include <baghash.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main (int argc, char *argv[])
{
  int xor_then_hash = false;
  long long int n_rounds = 3;
  long long int n_space = (1024*1024);
  long long int n_neighbors = 16;
  enum comp_method comp = 0;
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
          {"neighbors",  required_argument, 0, 'n'},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      char c = getopt_long (argc, argv, "xc:m:s:r:n:",
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
            comp = COMP__KECCAK_1600;
          else if (!strcmp (optarg, "blake2b"))
            comp = COMP__ARGON_BLAKE2B;
          else if (!strcmp (optarg, "sha512"))
            comp = COMP__SHA_512;
          else {
            fprintf (stderr, "Invalid compression method\n");
            return -1;
          }
          break;

        case 'm':
          if (!strcmp (optarg, "onebuffer"))
            mix = MIX__BAGHASH_ONE_BUFFER;
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

  printf ("NRounds        = %lld\n", (long long int)n_rounds);
  printf ("NSpace         = %lld\n", (long long int)n_space);
  printf ("Neighbs        = %lld\n", (long long int)n_neighbors);
  printf ("Mix            = %d\n", mix);
  printf ("Compression    = %d\n", comp);
  printf ("XOR-then-hash  = %d\n", xor_then_hash);
  printf ("Input          = %s\n", in);
  printf ("Salt           = %s\n", salt);

  struct baghash_options opts;
  opts.m_cost = n_space;
  opts.t_cost = n_rounds;
  opts.n_neighbors = n_neighbors;
  opts.comp = comp;
  opts.mix = mix;
  opts.xor_then_hash = xor_then_hash;

  unsigned char out[32];
  int error;
  if ((error = BagHash (out, 32, in, strlen (in), salt, strlen (salt), &opts))) {
    fprintf (stderr, "BagHash failed with error: %d\n", error);
    return -1;
  }

  return 0;
}

