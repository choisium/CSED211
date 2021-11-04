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
#include <limits.h>
#include "cachelab.h"

static int verbose_flag = 0;
static int help_flag = 0;
static int s = -1;
static int E = -1;
static int b = -1;
static char* t;

/*
    cache[set index][element index]
    LSB 1 bit is valid bit. Remainder is tag bits.
*/
static unsigned** cache_tag;
static int** cache_lru;


void parse_args(int argc, char* argv[]);
void usage();
void simulate();
void cache_controller(unsigned address, int* is_hit, int* is_evicted);

int main(int argc, char* argv[]) {

    parse_args(argc, argv);
    printf("s: %d, E: %d, b: %d, t: %s\n", s, E, b, t);

    simulate();
    return 0;
}

void parse_args(int argc, char* argv[]) {
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
                t = calloc(strlen(optarg) + 1, sizeof(char));
                strcpy(t, optarg);
                break;
            case 'h':
                help_flag = 1;
                usage();
                exit(EXIT_SUCCESS);
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
                exit(EXIT_FAILURE);
            default:
                abort ();
        }
    }
    if (s < 0 || E < 0 || b < 0 || t == NULL) {
        fprintf(stderr, "./csim: Missing required command line argument\n");
        usage();
        exit(EXIT_FAILURE);
    }
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

void cache_controller(unsigned address, int* is_hit, int* is_evicted) {
    static int current_time = 0;

    int set_index = (address >> b) % (1 << s);
    unsigned tag = address >> (s + b);
    int elem_index, ru, lru = INT_MAX;
    int evicted_index = -1;

    // miss -> load
    *is_evicted = 0;
    *is_hit = 0;

    for (elem_index = 0; elem_index < E; elem_index++) {
        ru = cache_lru[set_index][elem_index];
        if (lru > ru) {
            lru = ru;
            evicted_index = elem_index;
        }

        if (ru >= 0 && cache_tag[set_index][elem_index] == tag) {
            *is_hit = 1;
            break;
        }
    }

    if (*is_hit) { // cache hit
        cache_lru[set_index][elem_index] = current_time++;
    } else { // cache miss. need to update tag
        if (cache_lru[set_index][evicted_index] >= 0)
            *is_evicted = 1;
        cache_tag[set_index][evicted_index] = tag;
        cache_lru[set_index][evicted_index] = current_time++;
    }
}

void simulate() {
    int hit = 0, miss = 0, eviction = 0;
    char type;
    unsigned address, next_address;
    int size;
    int is_hit, is_evicted;
    int i, j, iter_count;

    FILE* trace = fopen(t, "r");
    if (trace == NULL) {
        fprintf(stderr, "%s: No such file or directory\n", t);
        exit(EXIT_FAILURE);
    }

    cache_tag = calloc(s, sizeof(unsigned *));
    cache_lru = calloc(s, sizeof(int *));

    for(i = 0; i < s; i++) {
        cache_tag[i] = calloc(E, sizeof(unsigned));
        cache_lru[i] = calloc(E, sizeof(int));
        for(j = 0; j < E; j++)
            cache_lru[i][j] = -1;
    }

    while (fscanf(trace, " %c %x,%d", &type, &address, &size) != EOF) {
        printf("%c %x,%d", type, address, size);

        iter_count = type == 'M'? 2 : 1;
        for (i = 0; i < iter_count; i++) {
            next_address = address - address % (1 << b);
            do {
                cache_controller(address, &is_hit, &is_evicted);
                if (is_hit) {
                    hit++;
                    printf(" hit");
                }
                else {
                    miss++;
                    printf(" miss");
                }
                if(is_evicted) {
                    eviction++;
                    printf(" eviction");
                }
                next_address += (1 << b);
            } while (next_address < address + size);
        }

        printf("\n");
    }

    for(i = 0; i < s; i++) {
        free(cache_tag[i]);
        free(cache_lru[i]);
    }

    fclose(trace);

    printSummary(hit, miss, eviction);
}