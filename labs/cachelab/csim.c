#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

typedef struct _cache_entries {
    uint8_t valid;
    uint8_t priority;
    uint64_t tag;
    // uint8_t *block;
} cache_entries;

typedef struct _cache_set {
    cache_entries *entries;
} cache_sets;

typedef struct _cache {
    cache_sets *sets; // The actual cache
    uint32_t s; // The number of sets in the cache
    uint32_t e; // The number of entries in each set
    uint32_t b; // The size of the block in each entry
} cache;

typedef struct _TIO {
    uint64_t Tag;
    uint64_t Index;
    uint64_t Offset;
} TIO;

// Cache initialization with specific sets, entries and block size
void cache_init(cache *c, uint32_t set, uint32_t entr, uint32_t block_size);

// Free all the allocated memory from cache
void cache_destroy(cache *c);

// Perform TIO breakdown from an address
void TIO_breakdown(TIO *t, uint64_t address, cache *c);

// parsing the data operation on the cache
void parse_data_op(cache *c, char *op);

// update the cache information from the corresponding operation
void cache_update(cache *c, TIO *t);

// When cache miss, choose victim cache line by LRU replacement policy
cache_entries *choose_victim(cache *c);

int main(int argc, char* argv[])
{
    if (argc < 9) {
        fprintf(stderr, "Not enough arguments!\n");
        return -1;
    }
    uint32_t set = atoi(argv[2]);
    uint32_t entry = atoi(argv[4]);
    uint32_t block = atoi(argv[6]);

    printf("There are %u sets, each set have %u entry"
           ", each entry have a block size of %u bytes.\n"
           , set, entry, block);
    
    cache L1;
    cache_init(&L1, set, entry, block);

    FILE *fd = fopen(argv[8], "r");
    if (!fd) {
        fprintf(stderr, "File not found!\n");
        return -1;
    }

    char str[30];

    while (NULL != fgets(str, 30, fd)) {
        printf("%s", str);
        parse_data_op(&L1, str);
    }

    cache_destroy(&L1);

    // printSummary(0, 0, 0);
    return 0;
}

void cache_init(cache *c, uint32_t set, uint32_t entry, uint32_t block_size)
{
    c->s = set;
    c->e = entry;
    c->b = block_size;
    c->sets = malloc(sizeof(cache_sets) * set);
    for (uint32_t i = 0U; i < set; i++) {
        c->sets[i].entries =  calloc(entry, sizeof(cache_entries));
        // for (uint32_t j = 0U; j < entry; j++) {
        //     c->sets[i].entries[j].block = malloc(sizeof(uint8_t) * block_size);
        // }
    }
}

void cache_destroy(cache *c)
{
    for (uint32_t i = 0U; i < c->s; i++) {
        // for (uint32_t j = 0U; j < c->e; j++) {
        //     free(c->sets[i].entries[j].block);
        // }
        free(c->sets[i].entries);
    }
    free(c->sets);
}

void TIO_breakdown(TIO *t, uint64_t address, cache *c)
{
    uint32_t index_bits = (uint32_t) log2(c->s);
    uint32_t offset_bits = (uint32_t) log2(c->b);
    uint32_t tag_bits = (uint32_t) (64U - index_bits - offset_bits);
    uint64_t offset_mask = (1UL << offset_bits) - 1UL;
    uint64_t index_mask = ((1UL << index_bits) - 1UL) << offset_bits;
    uint64_t tag_mask = 0xFFFFFFFFFFFFFFFFUL ^ (offset_mask | index_mask);
    t->Offset = address & offset_mask;
    t->Index = (address & index_mask) >> offset_bits;
    t->Tag = (address & tag_mask) >> (index_bits + offset_bits);
    printf("For address 0x%lx\n", address);
    printf("There are %u tag bits, %u index bits, %u offset bits\n", tag_bits, index_bits, offset_bits);
    printf("The tag bit is 0x%lx\n", t->Tag);
    printf("The index bit is 0x%lx\n", t->Index);
    printf("The offset bit is 0x%lx\n", t->Offset);
    printf("\n");
}

void parse_data_op(cache *c, char *op)
{
    char operation = op[1]; // store L, M or S
    char *addr_str = op + 3; // The pointer storing the operation address
    strtok(addr_str, ",");
    uint64_t addr_hex = strtol(addr_str, NULL, 16);
    // printf("%s\n", addr_str);
    // printf("%lx\n", addr_hex);

    switch (operation) {
        case 'L':
            printf("This is a load instruction\n");
            break;
        case 'M':
            printf("This is a modify instruction\n");
            break;
        case 'S':
            printf("This is a store instruction");
            break;
        default:
            printf("Unknown instruction!");
    }

    TIO t;
    TIO_breakdown(&t, addr_hex, c);

}

void cache_update(cache *c, TIO *t)
{
    uint8_t match_flag = 0U;
    for (uint32_t i = 0U; i < c->e; i++) {
        if ((c->sets[t->Index].entries[i].tag == t->Tag) 
        && (c->sets[t->Index].entries[i].valid = 1U)) // cache hit
            match_flag = 1U;
    }
    if (match_flag == 1U) // cache hit, return immediately
        return;
    else { // cache miss, need to update cache
        for ()
    }
}