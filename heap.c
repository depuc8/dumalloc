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

/* ── Static arena ─────────────────────────────────────────────────────────── */

/* The entire heap lives in this fixed-size byte array.  Giving it static
 * storage ensures it is zero-initialised before main() is entered. */
static char arr[PREALLOCATED_HEAP_SIZE];

/* ── Internal helpers ─────────────────────────────────────────────────────── */

/* Return a pointer to the first header in the arena. */
static size_t *heap_start(void)
{
    return (size_t *)arr;
}

/* Return the one-past-the-end sentinel of the arena. */
static size_t *heap_end(void)
{
    return (size_t *)(arr + PREALLOCATED_HEAP_SIZE);
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
        /* bit 0 of the header encodes the allocated flag; skip taken blocks. */
        if (!(*header & 1)) {
            if (*header >= size) {
                /* Found a free block large enough — return it. */
                return header;
            }

            /* Block is free but too small; try to merge with the next block. */
            next = (size_t *)((char *)header + *header);

            if (next < heap_end() && !(*next & 1)) {
                /* Next block is also free — absorb it and retry the size check
                 * without advancing 'header' (hence 'continue'). */
                *header += *next;
                continue;
            }
        }

        /* Advance to the next block.  Mask off the allocation flag to get the
         * true block size before computing the pointer arithmetic. */
        header = (size_t *)((char *)header + (*header & ~1UL));
    }

    return NULL; /* No suitable block found. */
}

/* ── Public allocator API ─────────────────────────────────────────────────── */

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

    /* Account for the header word and round up to alignment. */
    size_t total = ALIGN(size + SIZE_T_SZ);
    if (total < MIN_BLK_SZ)
        total = MIN_BLK_SZ;

    size_t *header = find_fit(total);
    if (!header)
        return NULL;

    /* Mark the block as allocated (set bit 0). */
    *header = (*header & ~1UL) | 1;

    /* Return a pointer to the payload, skipping past the header. */
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

    /* The header sits immediately before the payload pointer. */
    size_t *header = (size_t *)ptr - 1;

    /* Clear the allocation flag to mark the block as free. */
    *header &= ~1UL;
}
