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

typedef struct _hme {
    uint32_t hit;
    uint32_t miss;
    uint32_t evict;
} hme;

// Cache initialization with specific sets, entries and block size
void cache_init(cache *c, uint32_t set, uint32_t entr, uint32_t block_size);

// Free all the allocated memory from cache
void cache_destroy(cache *c);

// Perform TIO breakdown from an address
void TIO_breakdown(TIO *t, uint64_t address, cache *c);

// parsing the data operation on the cache
void parse_data_op(cache *c, char *op, hme *h);

// update the cache information from the corresponding operation
void cache_update(cache *c, TIO *t, hme *h);

// When cache miss, choose victim cache line by LRU replacement policy
cache_entries *choose_victim(cache *c, TIO *t, hme *h);

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
           , (uint32_t) pow(2, set), entry, (uint32_t) pow(2, block));
    
    cache L1;
    cache_init(&L1, set, entry, block);

    FILE *fd = fopen(argv[8], "r");
    if (!fd) {
        fprintf(stderr, "File not found!\n");
        return -1;
    }

    char str[30];
    hme result = {0, 0, 0};

    while (NULL != fgets(str, 30, fd)) {
        printf("%s", str);
        parse_data_op(&L1, str, &result);
    }

    cache_destroy(&L1);

    printSummary(result.hit, result.miss, result.evict);
    return 0;
}

void cache_init(cache *c, uint32_t set, uint32_t entry, uint32_t block_size)
{
    c->s = pow(2, set);
    c->e = entry;
    c->b = pow(2, block_size);
    c->sets = malloc(sizeof(cache_sets) * c->s);
    for (uint32_t i = 0U; i < c->s; i++) {
        c->sets[i].entries =  calloc(entry, sizeof(cache_entries));
    }
}

void cache_destroy(cache *c)
{
    for (uint32_t i = 0U; i < c->s; i++) {
        free(c->sets[i].entries);
    }
    free(c->sets);
}

void TIO_breakdown(TIO *t, uint64_t address, cache *c)
{
    uint32_t index_bits = (uint32_t) log2(c->s);
    uint32_t offset_bits = (uint32_t) log2(c->b);
    uint64_t offset_mask = (1UL << offset_bits) - 1UL;
    uint64_t index_mask = ((1UL << index_bits) - 1UL) << offset_bits;
    uint64_t tag_mask = 0xFFFFFFFFFFFFFFFFUL ^ (offset_mask | index_mask);

    t->Offset = address & offset_mask;
    t->Index = (address & index_mask) >> offset_bits;
    t->Tag = (address & tag_mask) >> (index_bits + offset_bits);
}

void parse_data_op(cache *c, char *op, hme *h)
{
    char operation = op[1]; // store L, M or S
    char *addr_str = op + 3; // The pointer storing the operation address
    strtok(addr_str, ",");
    uint64_t addr_hex = strtol(addr_str, NULL, 16);

    switch (operation) {
        case 'L':
            printf("This is a load instruction\n");
            break;
        case 'M':
            printf("This is a modify instruction\n");
            break;
        case 'S':
            printf("This is a store instruction\n");
            break;
        default:
            printf("Unknown instruction! Skip it\n");
            return;
    }

    TIO t;
    TIO_breakdown(&t, addr_hex, c);

    if (operation == 'M')
        cache_update(c, &t, h);
    cache_update(c, &t, h);
    printf("\n");

}

void cache_update(cache *c, TIO *t, hme *h)
{
    uint8_t hit_flag = 0U;
    cache_entries *chosen_line;
    for (uint32_t i = 0U; i < c->e; i++) {
        if ((c->sets[t->Index].entries[i].tag == t->Tag) 
        && (c->sets[t->Index].entries[i].valid == 1U)) { // cache hit
            chosen_line = &c->sets[t->Index].entries[i];
            hit_flag = 1U;
            h->hit += 1U;
            printf("Hit ");
        }
    }
    
    if (hit_flag == 0U) { // cache miss, need to update cache
        chosen_line = choose_victim(c, t, h);
        chosen_line->tag = t->Tag;
        chosen_line->valid = 1U;
    }

    for (uint32_t i = 0U; i < c->e; i++)
        c->sets[t->Index].entries[i].priority += 1U;
    chosen_line->priority = 0U;
}

cache_entries *choose_victim(cache *c, TIO *t, hme *h)
{
    for (uint32_t i = 0U; i < c->e; i++)
        if (c->sets[t->Index].entries[i].valid == 0U) { // if have non-valid, choose that
            h->miss += 1U;
            printf("Miss ");
            return &c->sets[t->Index].entries[i];
        }
    
    // If all the entries are valid, choose the least recently used
    uint32_t oldest_priority = 0U, oldest_entry = 0U;
    for (uint32_t i = 0U; i < c->e; i++) {
        if (c->sets[t->Index].entries[i].priority > oldest_priority) {
            oldest_priority = c->sets[t->Index].entries[i].priority;
            oldest_entry = i;
        }
    }
    printf("Evict ");
    h->miss += 1U;
    h->evict += 1U;
    return &c->sets[t->Index].entries[oldest_entry];
}