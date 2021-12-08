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
 * [Allocated Block]
 * 
 *
 * Prev pointer를 최대한 없애보려고 했는데, free list에서 remove를 할 때
 * prev 없으면 prev의 next pointer를 업데이트하기 어려울 듯
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"


/* Basic constants and macros */
#define WSIZE           4           /* Word and header/footer size (bytes) */
#define DSIZE           8           /* Double word size (bytes) */
#define OVERHEAD        8           /* Overhead for allocated block (bytes) */
#define MINBLOCKSIZE    24          /* Minimum block size (bytes) */
#define INITHEAPSIZE    (1 << 6)    /* Initial heap size (bytes) */
#define CHUNKSIZE       (1 << 12)   /* Extend heap by this amount (bytes) */
#define SIZECLASSNUM    20          /* The number of size class in segregated list */

#define MAX(x, y)       ((x) > (y)? (x): (y))
#define MIN(x, y)       ((x) < (y)? (x): (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)   ((size) | (alloc))

/* Get aligned size */
#define ALIGN(size)     ((size + (DSIZE - 1)) & ~0x7)

/* Read and write a word at address p */
#define GET(p)          (*(unsigned int *)(p))
#define PUT(p, val)     (*(unsigned int *)(p) = (int)(val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous block */
#define NEXT_BLKP(bp)   ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)   ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Given block ptr bp, compute address of its succ and pred pointer */
#define SUCC(bp)        ((char *)(bp))
#define PRED(bp)        ((char *)(bp) + DSIZE)

/* Given block ptr bp, compute address of its successor and predecessor block */
#define SUCC_BLKP(bp)   (*(char **)SUCC(bp))
#define PRED_BLKP(bp)   (*(char **)PRED(bp))


/* Pointer to initial heap space */
static char *heap_listp;
static char *free_listp;
static char *seg_list[SIZECLASSNUM];

/* Static helper functions */
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static void insert_block(void* bp);
static void delete_block(void* bp);
static int get_seg_index(size_t words);


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *) -1)
        return -1;
        
    /* Initialize prologue and epilogue */
    PUT(heap_listp, 0);                           /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));  /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));  /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));      /* Epilogue header */
    heap_listp += (2*WSIZE);

    free_listp = NULL;

    int i;
    for (i = 0; i < SIZECLASSNUM; i++) {
        seg_list[i] = NULL;
    }

    /* Extend the empty heap with a free block of CHUNKSIZE BYTES */
    if (extend_heap(INITHEAPSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;       /* Adjusted block size */
    size_t extendsize;  /* Amount to extend heap if no fit */
    char *bp;

    /* Ignore spurious request */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    asize = ALIGN(size + OVERHEAD);
    if (asize <= MINBLOCKSIZE)
        asize = MINBLOCKSIZE;

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);

    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldbp = ptr;  /* Old block pointer */
    void *newbp;        /* New block pointer */
    size_t oldSize;     /* Old block size */
    size_t asize;       /* Adjusted block size */
    size_t copysize;    /* Adjusted block size */

    /* Handle extreme case */
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    if (ptr == NULL) {
        return mm_malloc(size);
    }

    /* Compare old block size and requested size */
    oldSize = GET_SIZE(HDRP(ptr));
    asize = ALIGN(size + OVERHEAD);
    if (asize <= oldSize)
        /* When requested size is adaptable, just return the pointer */
        return ptr;
    else
        /*
         * When requested size is not feasible, allocate twice of them
         * to optimize performance for realloc traces
         */
        asize *= 1.5;
    
    /* Allocate block for requested size */
    newbp = mm_malloc(asize);
    if (newbp == NULL)
        return NULL;

    /* Copy contents */
    copysize = GET_SIZE(HDRP(oldbp));
    if (size < copysize)
        copysize = size;
    
    memcpy(newbp, oldbp, copysize);
    mm_free(oldbp);
    return newbp;
}

/*
 * extend_heap - Extend the heap with a new free block
 */
static void *extend_heap(size_t words)
{
    void *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    
    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));           /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));           /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));   /* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/*
 * coalesce - Join successive free block
 */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {         /* Case 1 */
        insert_block(bp);
    }

    else if (prev_alloc && !next_alloc) {   /* Case 2 */
        delete_block(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        insert_block(bp);
    }

    else if (!prev_alloc && next_alloc) {   /* Case 3 */
        delete_block(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        insert_block(bp);
    }

    else {                                  /* Case 4 */
        delete_block(NEXT_BLKP(bp));
        delete_block(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        insert_block(bp);
    }

    return bp;
}

/*
 * find_fit - first-fit search of the explicit free list
 */
static void *find_fit(size_t asize)
{
    /* First-fit search */
    void *bp;
    int seg_index = get_seg_index(asize/WSIZE);

    for (; seg_index < SIZECLASSNUM; seg_index++) {
        for (bp = seg_list[seg_index]; bp != NULL; bp = SUCC_BLKP(bp)) {
            if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
                return bp;
            }
        }
    }

    return NULL;    /* No fit */
}

/*
 * place - place requested block at the beginning of the free block
 * splitting only if the size of the remainder would equal or exceed
 * the minimum block size.
 */
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= MINBLOCKSIZE) {
        delete_block(bp);
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));

        /* Split */
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK((csize-asize), 0));
        PUT(FTRP(bp), PACK((csize-asize), 0));
        insert_block(bp);
    }
    else {
        delete_block(bp);
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

static void insert_block(void* bp) {
    int seg_index = get_seg_index(GET_SIZE(HDRP(bp))/WSIZE);

    /* Update next pointer */
    if (seg_list[seg_index] == NULL)
    {
        seg_list[seg_index] = bp;
        PUT(SUCC(bp), NULL);
        PUT(PRED(bp), NULL);
    }
    else
    {
        /* Update pointer in bp */
        PUT(SUCC(bp), seg_list[seg_index]);
        PUT(PRED(bp), NULL);
        /* Update pointer next to bp */
        PUT(PRED(seg_list[seg_index]), bp);
        seg_list[seg_index] = bp;
    }
}

static void delete_block(void* bp) {
    int seg_index = get_seg_index(GET_SIZE(HDRP(bp))/WSIZE);

    if (SUCC_BLKP(bp) != NULL) {   /* bp is not tail */
        PUT(PRED(SUCC_BLKP(bp)), PRED_BLKP(bp));
    }

    if (PRED_BLKP(bp) != NULL) {   /* bp is not head */
        PUT(SUCC(PRED_BLKP(bp)), SUCC_BLKP(bp));
    } else {                    /* bp is head */
        seg_list[seg_index] = SUCC_BLKP(bp);
    }
}

static int get_seg_index(size_t words) {
    int i;
    int class_size = 2;
    for (i = 0; i < SIZECLASSNUM; i++) {
        class_size <<= 1;
        if (words < class_size) {
            break;
        }
    }
    return MIN(i, SIZECLASSNUM - 1);
}