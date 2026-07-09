/*
 * heap.c — Core allocator implementation for dumalloc.
 *
 * Uses an implicit free-list over a statically allocated arena.
 * Each block begins with a size_t header whose value encodes two things:
 *   - bits[N..1]  : total block size in bytes (including the header)
 *   - bit[0]      : allocation flag — 1 = allocated, 0 = free
 *
 * find_fit() performs a first-fit search and opportunistically coalesces
 * adjacent free blocks while scanning, reducing fragmentation.
 */

#include <stddef.h>
#include "heap.h"

static char arr[PREALLOCATED_HEAP_SIZE];
static int  heap_ready = 0;

static size_t *heap_start(void)
{
    return (size_t *)arr;
}

static size_t *heap_end(void)
{
    return (size_t *)(arr + PREALLOCATED_HEAP_SIZE);
}

/*
 * heap_init — write the initial free-block sentinel.
 *
 * The arena is zero-initialised by the C runtime, but a zero header value
 * causes find_fit to loop forever (size 0 never advances the scan pointer).
 * Writing a single free block of PREALLOCATED_HEAP_SIZE bytes at the start
 * of the arena gives find_fit a valid list to walk.
 */
void heap_init(void)
{
    *heap_start() = PREALLOCATED_HEAP_SIZE; /* one big free block */
    heap_ready = 1;
}

/*
 * find_fit — first-fit search with inline coalescing.
 *
 * Scans the block list from the start of the arena looking for a free block
 * whose size is at least 'size' bytes.  When it finds a free block that is
 * too small, it checks whether the immediately following block is also free
 * and, if so, merges the two before retrying.
 *
 * Returns a pointer to the header of a suitable block, or NULL if no block
 * large enough exists.
 */
void *find_fit(size_t size)
{
    size_t *header = heap_start();
    size_t *next;

    while (header < heap_end()) {
        if (!(*header & 1)) {
            if (*header >= size)
                return header;

            /* Block is free but too small; try coalescing with the next block. */
            next = (size_t *)((char *)header + *header);

            if (next < heap_end() && !(*next & 1)) {
                *header += *next;
                continue;
            }
        }

        header = (size_t *)((char *)header + (*header & ~1UL));
    }

    return NULL;
}

/*
 * my_malloc — allocate 'size' bytes from the heap.
 *
 * Rounds 'size' up to the alignment boundary, searches for a free block via
 * find_fit(), marks it as allocated, and returns a pointer to the payload
 * (i.e. the byte immediately after the header).
 *
 * Returns NULL if the heap is exhausted or size is 0.
 */
void *my_malloc(size_t size)
{
    if (size == 0)
        return NULL;

    if (!heap_ready)
        heap_init();

    size_t total = ALIGN(size + SIZE_T_SZ);
    if (total < MIN_BLK_SZ)
        total = MIN_BLK_SZ;

    size_t *header = find_fit(total);
    if (!header)
        return NULL;

    size_t block_size = *header; /* free block's full size */

    if (block_size - total >= MIN_BLK_SZ) {
        /* Split: write a free header for the leftover region. */
        size_t *next = (size_t *)((char *)header + total);
        *next = block_size - total; /* free, no flag bit */
        *header = total | 1;        /* this block: allocated */
    } else {
        /* Remainder too small to be useful — absorb the whole block. */
        *header = (*header & ~1UL) | 1;
    }

    return (void *)(header + 1);
}

/*
 * my_free — release a previously allocated block.
 *
 * Clears the allocation flag in the block's header.  Adjacent free blocks
 * will be coalesced lazily the next time find_fit() scans past them.
 *
 * Passing NULL is a safe no-op, matching standard free() semantics.
 */
void my_free(void *ptr)
{
    if (!ptr)
        return;

    size_t *header = (size_t *)ptr - 1;
    *header &= ~1UL;
}
