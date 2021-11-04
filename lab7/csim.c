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
#include <string.h>
#include "cachelab.h"

static int verbose_flag = 0;
static int help_flag = 0;
static int s = -1;
static int E = -1;
static int b = -1;
static char* trace;

int parse_args(int argc, char* argv[]);
void usage();

int main(int argc, char* argv[]) {
    int ret;

    ret = parse_args(argc, argv);
    if (ret != 0) return ret;
    if (help_flag) return 0;

    printf("s: %d, E: %d, b: %d, t: %s\n", s, E, b, trace);

    printSummary(0, 0, 0);
    return 0;
}


int parse_args(int argc, char* argv[]) {
    int c;

    opterr = 0;

    while ((c = getopt (argc, argv, "hvs:E:b:t:")) != -1) {
        switch (c) {
            case 'v':
                verbose_flag = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                trace = calloc(strlen(optarg) + 1, sizeof(char));
                strcpy(trace, optarg);
                break;
            case 'h':
                help_flag = 1;
                usage();
                return 0;
            case '?':
                if (optopt == 'c')
                fprintf (stderr, "./csim: Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                fprintf (stderr, "./csim: Unknown option `-%c'.\n", optopt);
                else
                fprintf (stderr,
                        "./csim: Unknown option character `\\x%x'.\n",
                        optopt);
                usage();
                return 1;
            default:
                abort ();
        }
    }
    if (s == -1 || E == -1 || b == -1 || trace == NULL) {
        fprintf(stderr, "./csim: Missing required command line argument\n");
        usage();
        return 1;
    }
    return 0;
}


void usage() {
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\n");
    printf("Examples:\n");
    printf("  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}