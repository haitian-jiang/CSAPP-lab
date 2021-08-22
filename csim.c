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


uint8_t verbose = 0;
uint64_t s, b, E, S, B;
char* trace_file;
cache_t cache;
uint64_t hit_cnt, miss_cnt, evict_cnt, lru_cnt;


void print_help(char * const *);
void parse_args(int, char * const *);
void init_cache();
void free_cache();
void replay(char *);
void add_data(mem_addr_t);


int main(int argc, char* const argv[]) {
    parse_args(argc, argv);
    init_cache();
    replay(trace_file);
    free_cache();
    printSummary(hit_cnt, miss_cnt, evict_cnt);
    return 0;
}


void print_help(char * const argv[]) {
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


void parse_args(int argc, char * const argv[]) {
    char opt;
    while((opt = getopt(argc, argv, "s:b:E:t:hv")) != EOF) {
        switch (opt) {
            case 'h':
                print_help(argv);
                exit(0);
            case 'v':
                verbose = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 't':
                trace_file = optarg;
                break;
        }
    }
    if (!s || !b || !E || !trace_file) {
        fprintf(stderr, "%s: Please enter command line arguments!\n", argv[0]);
        print_help(argv);
        exit(1);
    }
}


void init_cache() {
    S = 1 << s; B = 1 << b;
    cache = (cache_t) malloc(S * sizeof(cache_set_t));
    for (int i = 0; i < S; ++i) {
        cache[i] = (cache_set_t) malloc(E * sizeof(cache_line_t));
        for (uint64_t j = 0; j < E; ++j) {
            cache[i][j].valid = 0;
            cache[i][j].tag = 0;
            cache[i][j].lru = 0;
        }
    }
}


void free_cache() {
    for (int i = 0; i < S; ++i) {
        free(cache[i]);
    }
    free(cache);
}


void replay(char* trace_file) {
    FILE * fp = NULL;
    fp = fopen(trace_file, "r");
    if (fp == NULL) {
        perror(trace_file);
        exit(1);
    } 
    char buf[100];
    while(fgets(buf, 90, fp) != NULL) {
        if (buf[0] == 'I') {
            continue;
        }
        mem_addr_t addr; uint32_t size;
        sscanf(buf+3, "%lx,%u", &addr, &size);
        if (verbose) {
            printf("%c %lx,%u ", buf[1], addr, size);
        }
        switch (buf[1]) {
            case 'M':
                add_data(addr);
            case 'L':
            case 'S':
                add_data(addr);
        }
        if (verbose) {
            printf("\n");
        }
    }
}


void add_data(mem_addr_t addr) {
    cache_set_t cache_set = cache[ (addr >> b) & (S - 1) ];
    uint64_t tag = addr >> (s + b);
    for (int i = 0; i < E; ++i) {
        if (tag == cache_set[i].tag && cache_set[i].valid) {
            ++hit_cnt;
            if (verbose){
                printf("hit ");
            }
            cache_set[i].lru = lru_cnt++;
            return;
        }
    }
    ++miss_cnt;
    if (verbose) {
        printf("miss ");
    }
    uint32_t min_lru = -1, line; 
    for (int i = 0; i < E; ++i) {
        if (cache_set[i].lru < min_lru) {
            line = i;
            min_lru = cache_set[i].lru;
        }
    }
    if (cache_set[line].valid) {
        ++evict_cnt;
        if (verbose) {
            printf("eviction ");
        }
    }
    cache_set[line].valid = 1;
    cache_set[line].tag = tag;
    cache_set[line].lru = lru_cnt++;
}
