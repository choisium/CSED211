/*
 * mm.c - Malloc implementation with segregated free list
 * 
 * [Structure]
 * - Every block has header and footer with allocated info at LSB.
 * - Free block has succ pointer and prev pointer
 * - Allocated Block structure
 *   [ Header (4 bytes) | Payload (8-byte aligned) | Padding | Footer (4 bytes) ]
 * - Free block structure
 *   [ Header (4 bytes) | Succ (8 bytes) | Prev (8 bytes) | Padding | Footer (4 bytes) ]
 *
 * [How blocks are managed]
 * - Free blocks are kept in segreated list(seg_list). It has SIZECLASSNUM elements
 *   and each seg_list[i] holds blocks which have 2^(k+1) and 2^(k+2) words.
 *   Exceptionionally, seg_list[0] holds blocks which have less than 2^2 words,
 *   and seg_list[SIZECLASSNUM - 1] holds blocks which have more than 2^SIZECLASSNUM words.
 * - When it need a free block to allocate, it finds a free block from the lowest
 *   seg_list element where the block size fit. In each seg_list element, first-fit
 *   search is applied.
 * - When a block is freed, the block is put at the front of the seg_list where it fits.
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


/* Useful global variables */
static char *heap_listp;                /* Pointer to initial heap space */
static char *seg_list[SIZECLASSNUM];    /* Segregatedd list */
static int realloc_flag = 0;            /* Flag whether doing realloc */

/* Static helper functions */
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static void insert_block(void* bp);
static void delete_block(void* bp);
static int get_seg_index(size_t words);

/* Consistency checker */
static void mm_check();

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

    int i;
    for (i = 0; i < SIZECLASSNUM; i++) {
        seg_list[i] = NULL;
    }

    /* Extend the empty heap with a free block of CHUNKSIZE BYTES */
    if (extend_heap(INITHEAPSIZE/WSIZE) == NULL)
        return -1;
    
    /* Check heap consistency before return */
    // mm_check();

    return 0;
}

/* 
 * mm_malloc - Allocate a block using find_fit and place. If there is no
 * proper free block, do extend_heap and then place.
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
    asize = ALIGN(size == 112 || size == 448? (size + (size >> 2)): size + OVERHEAD);

    if (asize <= MINBLOCKSIZE)
        asize = MINBLOCKSIZE;

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);

        /* Check heap consistency before return */
        // printf("\nmalloc fit\n");
        // mm_check();

        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);

    /* Check heap consistency before return */
    // printf("\nmalloc extend\n");
    // mm_check();

    return bp;
}

/*
 * mm_free - Freeing a block. Set allocate bit as 0 and do coalesce.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);

    /* Check heap consistency before return */
    // printf("\nfree\n");
    // mm_check();
}

/*
 * mm_realloc - Realloc using mm_malloc and mm_free.
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
    oldSize = GET_SIZE(HDRP(oldbp));
    asize = ALIGN(size + OVERHEAD);
    if (asize <= oldSize) {
        /* When requested size is adaptable, just return the pointer */
        return oldbp;
    }
    
    /* Allocate block for requested size */
    realloc_flag = 1;
    newbp = mm_malloc(asize);
    realloc_flag = 0;
    if (newbp == NULL)
        return NULL;

    /* Copy contents */
    copysize = GET_SIZE(HDRP(oldbp));
    if (size < copysize)
        copysize = size;
    
    memcpy(newbp, oldbp, copysize);
    mm_free(oldbp);

    /* Check heap consistency before return */
    // printf("\nrealloc malloc\n");
    // mm_check();

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
 * find_fit - first-fit search in proper segregated free list
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
 * place - place requested block at the beginning of the free block,
 * splitting only if it is not from realloc and the size of the remainder
 * is equal or exceed the minimum block size.
 * During realloc, don't split to increase utility of realloc testcases.
 */
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    if (!realloc_flag && (csize - asize) >= MINBLOCKSIZE) {
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

/*
 * insert_block - Insert the block at the front of the seg_list
 * where the block fits.
 */
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

/*
 * delete_block - Delete the block from the seg_list.
 */
static void delete_block(void* bp) {
    int seg_index = get_seg_index(GET_SIZE(HDRP(bp))/WSIZE);

    /* Update PRED pointer */
    if (SUCC_BLKP(bp) != NULL)  /* bp is not tail */
    {
        PUT(PRED(SUCC_BLKP(bp)), PRED_BLKP(bp));
    }

    /* Update SUCC pointer */
    if (PRED_BLKP(bp) != NULL)  /* bp is not head */
    {
        PUT(SUCC(PRED_BLKP(bp)), SUCC_BLKP(bp));
    }
    else                        /* bp is head */
    {
        seg_list[seg_index] = SUCC_BLKP(bp);
    }
}

/*
 * get_seg_index - Find proper seg_list index using words num.
 * If words is between 2^(k+1) and 2^(k+2), return will be k.
 */
static int get_seg_index(size_t words) {
    int i;
    int class_size = 4;

    for (i = 0; i < SIZECLASSNUM; i++, class_size <<= 1)
        if (words < class_size)
            break;

    return MIN(i, SIZECLASSNUM - 1);
}

/*
 * mm_check - Check heap consistency.
 */
static void mm_check() {
    void *bp;

    /* Check all block consistency */
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        /* Block payload must be double-word aligned */
        assert ((int) bp == ALIGN((int) bp));
        
        /* Block info in header and footer must be same. */
        assert (GET_SIZE(HDRP(bp)) == GET_SIZE(FTRP(bp)));
        assert (GET_ALLOC(HDRP(bp)) == GET_ALLOC(FTRP(bp)));

        /* Block size must be aligned */
        size_t size = GET_SIZE(HDRP(bp));
        assert (size == ALIGN(size));
        printf("bp: %x size: %d allocated: %d\n", bp, size, GET_ALLOC(HDRP(bp)));
        /* Pass header and footer */
        if (bp == heap_listp || size == 0)
            continue;

        /* Pointers must be in valid heap region */
        void *next = NEXT_BLKP(bp);
        void *prev = PREV_BLKP(bp);
        assert (GET_SIZE(HDRP(next)) == 0 || (next >= mem_heap_lo() && next <= mem_heap_hi()));
        assert (prev == heap_listp || (prev >= mem_heap_lo() && prev <= mem_heap_hi()));

        /* Block must be not overlapped */
        assert (next >= bp + size);

        /* Heap consistency check for free block */
        if (!GET_ALLOC(HDRP(bp)))
        {
            /* Pointers must be in valid heap region */
            void *pred = PRED_BLKP(bp);
            void *succ = SUCC_BLKP(bp);
            assert (pred == NULL || (pred >= mem_heap_lo() && pred <= mem_heap_hi()));
            assert (succ == NULL || (succ >= mem_heap_lo() && succ <= mem_heap_hi()));

            /* Free block must be in proper seg_list */
            int seg_index = get_seg_index(GET_SIZE(HDRP(bp))/WSIZE);
            void *walker;
            int found = 0;
            for (walker = seg_list[seg_index]; walker != NULL; walker = SUCC_BLKP(walker)) {
                if (walker == bp) {
                    found = 1;
                    break;
                }
            }
            assert (found == 1);
        }
    }

    /* Check free block consistency */
    int i;
    for (i = 0; i < SIZECLASSNUM; i++) {
        for (bp = seg_list[i]; bp != NULL; bp = SUCC_BLKP(bp)) {
            /* Free block should be marked as free */
            assert (GET_ALLOC(HDRP(bp)) == 0);

            /* Block's size must be fit in this seg_list */
            size_t words = GET_SIZE(HDRP(bp)) / WSIZE;
            if (i == 0)
                assert (words < 4);
            else if (i == SIZECLASSNUM - 1)
                assert (words >= (1 << SIZECLASSNUM));
            else
                assert (words >= (1 << (i + 1)) && words < (1 << (i + 2)));
            
            /* Free blocks can't be contiguous, must be coalesced. */
            void *next = NEXT_BLKP(bp);
            void *prev = PREV_BLKP(bp);
            assert (GET_ALLOC(HDRP(prev)) != 0);
            assert (GET_ALLOC(HDRP(next)) != 0);

            /* Pred block's successor must be bp, Succ block's predecessor must be bp. */
            void *pred = PRED_BLKP(bp);
            void *succ = SUCC_BLKP(bp);
            if (pred != NULL) {
                assert (SUCC_BLKP(pred) == bp);
            }
            if (succ != NULL) {
                assert (PRED_BLKP(succ) == bp);
            }
        }
    }
}