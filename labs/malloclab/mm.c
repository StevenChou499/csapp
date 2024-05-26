/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "WaterBurBur",
    /* First member's full name */
    "Steven Tsou",
    /* First member's email address */
    "stevenchou499@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE             4
#define DSIZE             8
#define CHUNKSIZE         (1U << 12)

typedef unsigned long long ull;

#define PACK(size, alloc) ((size) | (alloc))

#define GET(p)            (*(ull *)(p))
#define PUT(p, val)       (*(ull *)(p) = (val))

#define BLOCK_SIZE(p)      (*((ull *)(p) - 1) & ~0x7)
#define GET_SIZE(p)        (*(ull *)(p) & ~0x7)
#define GET_ALLOC(p)       (*(ull *)(p) & 0x1)
#define NEXT_BLOCK(p)       ((p) + BLOCK_SIZE(p))
#define PREV_BLOCK(p)       ((p) - GET_SIZE(p - 2 * DSIZE))

#define HDRP(p)          ((ull *)(p) - 1)
#define FTRP(p)          ((ull *)(p) + BLOCK_SIZE(p) / DSIZE - 2)

#define NXTFREEBLK(p)     ((char*)*(ull *)(p))
#define PRVFREEBLK(p)     ((char*)*(ull *)(p + DSIZE))

char *heap_start;
char *heap_end;

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    heap_start = (char *)mem_sbrk(5 * DSIZE);
    heap_end = heap_start + (5 * DSIZE);
    PUT(heap_start, PACK(4 * DSIZE, 1));
    heap_start += DSIZE;
    PUT(heap_start, (ull)heap_start);
    PUT(heap_start + 1 * DSIZE, (ull)heap_start);
    PUT(heap_start + 2 * DSIZE, PACK(4 * DSIZE, 1));
    PUT(heap_start + 3 * DSIZE, PACK(0, 1));

    char* free_block = NULL;
    free_block = (char *)mem_sbrk(CHUNKSIZE);
    heap_end += CHUNKSIZE;
    PUT(free_block - DSIZE, PACK(CHUNKSIZE, 0));
    PUT(free_block, (ull)heap_start);
    PUT(free_block + DSIZE, (ull)heap_start);
    PUT(FTRP(free_block), PACK(CHUNKSIZE, 0));
    PUT(heap_end - DSIZE, PACK(0, 1));

    PUT(heap_start, (ull)free_block);
    PUT(heap_start + DSIZE, (ull)free_block);
    
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    // printf("The size of SIZE_T_SIZE is %d bytes.\n", SIZE_T_SIZE);
    int newsize = ALIGN(size);
    // printf("The actual allocated bytes is %d bytes.\n", newsize);
    newsize = (newsize < 16) ? 16 : newsize;
    newsize += 2 * SIZE_T_SIZE;
    // printf("The added header and footer size is total %d bytes.\n", newsize);
    char* prev = heap_start, * curr = NXTFREEBLK(heap_start);
    while (curr != heap_start) {
        int curr_blk_size = BLOCK_SIZE(curr);
        if (curr_blk_size >= (newsize + (4 * SIZE_T_SIZE))) { // split for large enough block
            PUT(FTRP(curr), PACK(curr_blk_size - newsize, 0));
            PUT(HDRP(curr), PACK(newsize, 1));
            PUT(FTRP(curr), PACK(newsize, 1));
            PUT(HDRP(NEXT_BLOCK(curr)), PACK(curr_blk_size - newsize, 0));
            char *next_block = NXTFREEBLK(curr);
            // printf("The next block address is %p\n", (ull)next_block);
            *(ull *)next_block = *(ull *)curr;
            *((ull *)next_block + 1) = *((ull *)curr + 1);
            *((ull *)NXTFREEBLK(next_block) + 1) = (ull)next_block;
            *(ull *)PRVFREEBLK(next_block) = (ull)next_block;
            // printf("The footer addr is %p, the prev of next block is %p\n", FTRP(curr), (char*)(next_block - 2 * DSIZE));
            // printf("The size of previous block is %d\n", GET_SIZE(next_block - 2 * DSIZE));
            printf("Choosing starting address: 0x%llX\n", (ull)curr);
            return curr;
        }
        else if (curr_blk_size >= newsize) { // don't split block
            // set the header and footer as allocated
            PUT(HDRP(curr), PACK(curr_blk_size, 1));
            PUT(FTRP(curr), PACK(curr_blk_size, 1));
            char *next_block = NXTFREEBLK(curr);
            // printf("The next block address is %p\n", (ull)next_block);
            *(ull *)next_block = *(ull *)curr;
            *((ull *)next_block + 1) = *((ull *)curr + 1);
            *((ull *)NXTFREEBLK(next_block) + 1) = (ull)next_block;
            *(ull *)PRVFREEBLK(next_block) = (ull)next_block;
            printf("Choosing starting address: 0x%llX\n", (ull)curr);
            return curr; // return the pointer
        }
        else {
            prev = curr;
            curr = NXTFREEBLK(curr);
        }
    }
    if (curr == heap_start) { // current heap size isn't large enough, get another block
        // do something
        printf("Allocating more heap space...\n");
        char *new_mem = mem_sbrk(CHUNKSIZE);
        printf("new heap start address: 0x%llX\n", (ull)new_mem);
        char *last_block = PRVFREEBLK(heap_start);
        *((ull *)last_block) = (ull)new_mem;
        *((ull *)heap_start + 1) = (ull)new_mem;
        *(ull *)new_mem = (ull)heap_start;
        *((ull *)new_mem + 1) = (ull)last_block;
        PUT(HDRP(new_mem), PACK(CHUNKSIZE, 0));
        PUT(FTRP(new_mem), PACK(CHUNKSIZE, 0));
        PUT(HDRP(NEXT_BLOCK(new_mem)), PACK(0, 1));
        char *new = mm_malloc(size);
        // printf("Choosing starting address: 0x%llX\n", (ull)new);
        return new;
    }
    printf("mallocing %d bytes...\n", newsize);
    mm_info();
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    char *curr = (char *)ptr;
    char *prev = PREV_BLOCK(curr);
    char *next = NEXT_BLOCK(curr);
    ull prev_block_size = GET_SIZE(HDRP(PREV_BLOCK((char *)ptr)));
    ull freed_size = GET_SIZE(HDRP((char *)ptr));
    ull next_block_size = GET_SIZE(HDRP(NEXT_BLOCK((char *)ptr)));

    ull prev_status = GET_ALLOC((ull *)ptr - 2);
    ull next_status = GET_ALLOC(HDRP(NEXT_BLOCK((char *)ptr)));
    if ((prev_status == 0UL) && (next_status == 0UL)) { // merge both block
        // do both
        ull total_size = prev_block_size + freed_size + next_block_size;
        *((ull *)NXTFREEBLK(next) + 1) = (ull)prev;
        *(ull *)prev = *(ull *)next;
        PUT(HDRP(prev), PACK(total_size, 0));
        PUT(FTRP(next), PACK(total_size, 0));
        // *(ull *)prev = *(ull *)next;
        // *((ull *)NXTFREEBLK(next) + 1) = (ull)prev;
    } else if (prev_status == 0UL) { // merge previous block
        // do something
        // printf("The previous block address is %llx: \n", (ull)prev);
        // printf("The prev prev is 0x%llx, prev next is 0x%llX\n", *((ull *)prev + 1), *(ull *)prev);
        // printf("The current block address is %llx: \n", (ull)curr);
        // printf("The curr prev is 0x%llx, curr next is 0x%llX\n", *((ull *)curr + 1), *(ull *)curr);
        ull total_size = prev_block_size + freed_size;
        // *(ull *)prev = *(ull *)curr;
        // *(ull *)PRVFREEBLK(curr) = (ull)prev;
        // *((ull *)NXTFREEBLK(prev) + 1) = (ull)prev;
        PUT(HDRP(prev), PACK(total_size, 0));
        PUT(FTRP(curr), PACK(total_size, 0));
    } else if (next_status == 0UL) { // merge next block
        // do something
        ull total_size = freed_size + next_block_size;
        // *(ull *)curr = *(ull *)next;
        *(ull *)PRVFREEBLK(next) = (ull)curr;
        *((ull *)NXTFREEBLK(next) + 1) = (ull)curr;
        *(ull *)curr = *(ull *)next;
        *((ull *)curr + 1) = *((ull *)next + 1);
        // *((ull *)NXTFREEBLK(next) + 1) = (ull)curr;
        PUT(HDRP(curr), PACK(total_size, 0));
        PUT(FTRP(next), PACK(total_size, 0));
    } else { // no need to merge
        // do nothing
        PUT(HDRP((char *)ptr), PACK(freed_size, 0));
        PUT(FTRP((char *)ptr), PACK(freed_size, 0));
        *(ull *)ptr = *(ull *)heap_start;    // freed blk next blk is strt next blk
        *((ull *)ptr + 1) = (ull)heap_start; // freed blk prev blk is strt blk
        *((ull *)NXTFREEBLK(heap_start) + 1) = (ull)ptr; // 2nd blk prev blk is freed blk
        *(ull *)heap_start = (ull)ptr;       // strt blk next blk is freed blk
    }
    // ull new_status = PACK(GET_SIZE(HDRP((char *)ptr)), 0);
    // PUT(HDRP((char *)ptr), new_status);
    // PUT(FTRP((char *)ptr), new_status);
    // *(ull *)ptr = *(ull *)heap_start;    // freed blk next blk is strt next blk
    // *((ull *)ptr + 1) = (ull)heap_start; // freed blk prev blk is strt blk
    // *((ull *)NXTFREEBLK(heap_start) + 1) = (ull)ptr; // 2nd blk prev blk is freed blk
    // *(ull *)heap_start = (ull)ptr;       // strt blk next blk is freed blk

}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
   int original_size = GET_SIZE(HDRP(ptr));
    void *new_ptr = mm_malloc(size);
    memcpy(new_ptr, ptr, size);
    // for (int i = 0; i < original_size; i++) {
    //     *((char *)new_ptr + i) = *((char *)ptr + 1);
    // }
    mm_free(ptr);
    return new_ptr;
}


void mm_info()
{
    char* root = (char*)heap_start;
    printf("root info: 0x%llX, %d bytes\n", (ull) heap_start, BLOCK_SIZE(heap_start));
    printf("\tprev block: NONE, next block: 0x%llX\n", (ull)NEXT_BLOCK(heap_start));
    printf("\tAllocation status: %d\n", GET_ALLOC(HDRP(heap_start)));
    printf("\tprev free block: 0x%llX", *((ull *)root + 1));
    printf(", next free block: 0x%llX\n\n", *(ull *)root);
    root = NEXT_BLOCK(root);
    while (BLOCK_SIZE(root) != 0U) {
        printf("block info: 0x%llX, %d bytes\n", (ull)root, BLOCK_SIZE(root));
        // printf("The size of previous block is %d\n", GET_SIZE(root - 2 * DSIZE));
        printf("\tprev block: 0x%llX, next block: ", (ull)PREV_BLOCK(root));
        if (BLOCK_SIZE(NEXT_BLOCK(root)) == 0U) {
            printf("NONE");
        } else {
            printf("0x%llX", (ull)NEXT_BLOCK(root));
        }
        printf("\n\tAllocation status: %d\n", GET_ALLOC(HDRP(root)));
        if (GET_ALLOC(HDRP(root)) == 0U) {
            printf("\tprev free block: 0x%llX", *((ull *)root + 1));
            printf(", next free block: 0x%llX\n", *(ull *)root);
        }
        printf("\n");
        root = NEXT_BLOCK(root);
    }
}







