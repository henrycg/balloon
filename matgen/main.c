
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main (int argc, char *argv[])
{
  long long int n = 1024;
  long long int d = 48;

  while (1)
    {
      struct option long_options[] =
        {
          /* These options don’t set a flag.
             We distinguish them by their indices. */
          {"nodes",  required_argument, 0, 'n'},
          {"degree",  required_argument, 0, 'd'},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      char c = getopt_long (argc, argv, "d:n:",
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

        case 'n':
          errno = 0;
          n = strtoll (optarg, &end, 10);
          if (errno > 0 || *end != '\0' || n < 0) {
            fprintf (stderr, "Invalid argument to -n\n");
            return -1;
          }
          break;

        case 'd':
          errno = 0;
          d = strtoll (optarg, &end, 10);
          if (errno > 0 || *end != '\0' || d < 0) {
            fprintf (stderr, "Invalid argument to -d\n");
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

  printf ("// N = %lld", n);
  printf ("// d = %lld", d);
  return 0;
}

