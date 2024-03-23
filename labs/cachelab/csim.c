#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct _cache_entries {
    uint32_t valid;
    uint32_t tag;
    uint8_t *block;
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

// Cache initialization with specific sets, entries and block size
void cache_init(cache *c, uint32_t set, uint32_t entry, uint32_t block_size);

// Free all the allocated memory from cache
void cache_destroy(cache *c);

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
        for (uint32_t j = 0U; j < entry; j++) {
            c->sets[i].entries[j].block = malloc(sizeof(uint8_t) * block_size);
        }
    }
}

void cache_destroy(cache *c)
{
    for (uint32_t i = 0U; i < c->s; i++) {
        for (uint32_t j = 0U; j < c->e; j++) {
            free(c->sets[i].entries[j].block);
        }
        free(c->sets[i].entries);
    }
    free(c->sets);
}