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
#define FTRP(p)          ((ull *)(p) + BLOCK_SIZE(p) / DSIZE)

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
    int newsize = ALIGN(size + SIZE_T_SIZE);
    newsize = (newsize < 16) ? 16 : newsize;
    newsize += 2 * DSIZE;
    char* prev = heap_start, * curr = NXTFREEBLK(heap_start);
    while (curr != heap_start) {
        int curr_blk_size = BLOCK_SIZE(curr);
        if (curr_blk_size >= (newsize + 32)) { // split for large enough block
            PUT(FTRP(curr), PACK(curr_blk_size - newsize, 0));
            PUT(HDRP(curr), PACK(newsize, 1));
            PUT(FTRP(curr), PACK(newsize, 1));
            PUT(HDRP(NEXT_BLOCK(curr)), PACK(curr_blk_size - newsize, 0));
            char *next_block = NEXT_BLOCK(curr);
            *(ull *)next_block = *(ull *)curr;
            *((ull *)next_block + 1) = *((ull *)curr + 1);
            // if (BLOCK_SIZE(NXTFREEBLK(next_block)) != 0U)
                *((ull *)NXTFREEBLK(next_block) + 1) = (ull)next_block;
            // if (PRVFREEBLK(next_block) != heap_start)
                *(ull *)PRVFREEBLK(next_block) = (ull)next_block;
            return curr;
        }
        else if (curr_blk_size >= newsize) { // don't split block
            // set the header and footer as allocated
            PUT(HDRP(curr), PACK(curr_blk_size, 1));
            PUT(FTRP(curr), PACK(curr_blk_size, 1));
            return curr; // return the pointer
        }
        else {
            prev = curr;
            curr = NXTFREEBLK(curr);
        }
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}


void mm_info()
{
    char* root = (char*)heap_start;
    printf("root info: 0x%llX, %d bytes\n", (ull) heap_start, BLOCK_SIZE(heap_start));
    printf("\tprev block: NONE, next block: 0x%llX\n", (ull)NEXT_BLOCK(heap_start));
    printf("\tAlloction status: %d\n\n", GET_ALLOC(HDRP(heap_start)));
    root = NEXT_BLOCK(root);
    while (BLOCK_SIZE(root) != 0U) {
        printf("block info: 0x%llX, %d bytes\n", (ull)root, BLOCK_SIZE(root));
        printf("\tprev block: 0x%llX, next block: ", (ull)PREV_BLOCK(root));
        if (BLOCK_SIZE(NEXT_BLOCK(root)) == 0U) {
            printf("NONE");
        } else {
            printf("0x%llX", (ull)NEXT_BLOCK(root));
        }
        printf("\n\tAllocation status: %d\n\n", GET_ALLOC(root));
        root = NEXT_BLOCK(root);
    }
}







