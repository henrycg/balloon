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

#include "libballoon/constants.h"
#include "libballoon/errors.h"
#include "timing.h"

static void 
usage (const char *name)
{
  fprintf (stderr, "Usage: %s password\n", name);
  fprintf (stderr, "A test utility for the Balloon password-hashing function.\n\n");
  fprintf (stderr, "OPERATIONS\n");
  fprintf (stderr, "  -b, --blob=BLOB         Verify that the password matches\n"); 
  fprintf (stderr, "                            the BLOB passed as an argument. \n"); 
  fprintf (stderr, "  -h, --help              Print this help message.\n\n");
  fprintf (stderr, "PARAMETERS\n");
  fprintf (stderr, "  -t, --time=NUM          Number of mixing rounds.\n");
  fprintf (stderr, "                            Default = 1\n");
  fprintf (stderr, "  -s, --space=NUM         Space usage (in KB).\n");
  fprintf (stderr, "                            Default = 1024 KB\n");
  fprintf (stderr, "  -p, --parllelism=NUM    Number of threads to use.\n");
  fprintf (stderr, "                            Default = 1\n");
}

int 
main (int argc, char *argv[]) 
{
  int32_t t_cost = 1;
  int64_t s_cost = 1024;
  int16_t p_cost = 1;
  const char *verify_blob = NULL;
  int help = false;

  while (1)
    {
      struct option long_options[] =
        {
          /* These options don’t set a flag.
             We distinguish them by their indices. */
          {"time",  required_argument, 0, 't'},
          {"space",  required_argument, 0, 's'},
          {"parallelism",  required_argument, 0, 'p'},
          {"blob",  required_argument, 0, 'b'},
          {"help", no_argument, &help, 'h'},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      char c = getopt_long (argc, argv, "t:s:p:b:h?",
                       long_options, &option_index);
      char *end;

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
        {
        case 'b':
          verify_blob = optarg;
        case 0:
          /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0)
            break;
          break;

        case 's':
          errno = 0;
          s_cost = strtoll (optarg, &end, 10);
          if (errno > 0 || *end != '\0' || s_cost < 0) {
            fprintf (stderr, "Invalid argument to -s\n");
            return -1;
          }

          break;

        case 'p':
          errno = 0;
          p_cost = strtoll (optarg, &end, 10);
          if (errno > 0 || *end != '\0' || p_cost <= 0) {
            fprintf (stderr, "Invalid argument to -p\n");
            return -1;
          }
          break;

        case 't':
          errno = 0;
          t_cost = strtoll (optarg, &end, 10);
          if (errno > 0 || *end != '\0' || t_cost < 0) {
            fprintf (stderr, "Invalid argument to -t\n");
            return -1;
          }
          break;

        case 'h':
          help = true;
          break;
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
    if (optind + 1 < argc) {
        fprintf (stderr, "Too many arguments\n");
        return -1;
      }
    if (optind + 1 > argc)
      {
        fprintf (stderr, "Password not passed in\n");
        return -1;
      }
  }

  char *in = argv[optind];

  struct balloon_options opts = {
    .s_cost = s_cost,
    .t_cost = t_cost,
    .n_threads = p_cost
  };

  printf ("t_cost         = %u\n", opts.t_cost);
  printf ("s_cost         = %u\n", opts.s_cost);
  printf ("p_cost         = %u\n", opts.n_threads);
  printf ("passwd         = %s\n", in);

  int error;
  const double wall_start = wall_sec ();
  char blob[BLOB_LEN+1];

  if (verify_blob) {
    strncpy (blob, verify_blob, BLOB_LEN+1);

    if ((error = Balloon_Verify (blob, in, strlen (in)))) {
      fprintf (stderr, "Balloon_Hash failed with error: %s (%d)\n", error_to_string(error), error);
      return -1;
    }
  } else {
    if ((error = Balloon_Hash (blob, &opts, in, strlen (in)))) {
      fprintf (stderr, "Balloon_Hash failed with error: %s (%d)\n", error_to_string(error), error);
      return -1;
    }
  }
  const double wall_end = wall_sec ();
  const double wall_diff = wall_end - wall_start;
  printf("Time total      : %lg\n", wall_diff);
  printf("Hashes per sec  : %lg\n", (1.0f / wall_diff));
  printf("Output          : %s\n", blob);
  //write_hash (stdout, out, outlen, salt, saltlen, opts.m_cost, opts.t_cost);

  // Clean up OpenSSL junk
  EVP_cleanup();
  CRYPTO_cleanup_all_ex_data();
  ERR_remove_state(0);
  ERR_free_strings();
  return 0;
}

