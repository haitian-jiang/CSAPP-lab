#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "cachelab.h"

typedef uint64_t mem_addr_t;
typedef struct {
    char valid;
    mem_addr_t tag;
    uint32_t lru;
} cache_line_t;
typedef cache_line_t* cache_set_t;
typedef cache_set_t*  cache_t; 

uint32_t lru = -1;
uint8_t verbose = 0;
int s, b, E, S, B;
char* trace_file;
cache_t cache;

void print_help(char * const *);
void parse_args(int, char * const *);
void init_cache();

int main(int argc, char* const argv[])
{
    parse_args(argc, argv);
    init_cache();
    printSummary(0, 0, 0);
    return 0;
}

void print_help(char * const argv[]){
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n\n");
    printf("Examples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
}

void parse_args(int argc, char * const argv[]){
    char opt;
    while((opt = getopt(argc, argv, "s:b:E:t:hv")) != EOF){
        switch (opt){
            case 'h':
                print_help(argv);
                exit(0);
            case 'v':
                verbose = 1;
                break;
            case 's':
                s = atoi(optarg);
                printf("%d\n", s);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 't':
                trace_file = optarg;
        }
    }
    if (!s || !b || !E || !trace_file){
        fprintf(stderr, "%s: Please enter command line arguments!\n", argv[0]);
        print_help(argv);
        exit(1);
    }
}

void init_cache(){
    S = 2 << s; B = 2 << b;
    cache = (cache_t)malloc(S * sizeof(cache_set_t));
    for (int i = 0; i < S; ++i){
        cache[i] = (cache_set_t)malloc(E * sizeof(cache_line_t));
        for (int j = 0; j < E; ++j){
            cache[i][j].valid = 0;
            cache[i][j].tag = 0;
            cache[i][j].lru = 0;
        }
    }
}
