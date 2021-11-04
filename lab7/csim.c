/*
  Name: Choi Soomin
  Student ID: 20160169
  Login ID: choisium
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <unistd.h>
#include "cachelab.h"

int main(int argc, char* argv[])
{
    int c;

    opterr = 0;

    while ((c = getopt (argc, argv, "hvs:E:b:t:")) != -1) {
        switch (c) {
        case 'v':
            printf ("v flag, %s\n", optarg);
            break;
        case 's':
            printf ("s flag, %s\n", optarg);
            break;
        case 'E':
            printf ("E flag, %s\n", optarg);
            break;
        case 't':
            printf ("t flag, %s\n", optarg);
            break;
        case 'b':
            printf ("b flag, %s\n", optarg);
            break;
        case 'h':
            printf ("h flag, %s\n", optarg);
        case '?':
            if (optopt == 'c')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
            fprintf (stderr,
                    "Unknown option character `\\x%x'.\n",
                    optopt);
            return 1;
        default:
            abort ();
        }
    }

    printSummary(0, 0, 0);
    return 0;
}
