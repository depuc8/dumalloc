/*
 * heap.h — Public interface for dumalloc.
 *
 * Exposes constants, helper macros, and the two allocator entry-points
 * (my_malloc / my_free) that callers use to request and release heap memory.
 */
#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>

/* ── Alignment ────────────────────────────────────────────────────────────── */

/* All allocations are rounded up to a multiple of ALIGNMENT bytes so that
 * every returned pointer satisfies the strictest fundamental alignment
 * requirement on the target platform. */
#define ALIGNMENT 8

/* Round 'size' up to the next ALIGNMENT boundary. */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

/* Aligned size of a size_t value — used as the minimum header width. */
#define SIZE_T_SZ (ALIGN(sizeof(size_t)))

/* ── Heap configuration ───────────────────────────────────────────────────── */

/* Total number of bytes in the statically allocated heap arena. */
#define PREALLOCATED_HEAP_SIZE 16384

/* Smallest meaningful block size (header + at least ALIGNMENT bytes of
 * payload).  Blocks smaller than this are never split off. */
#define MIN_BLK_SZ (ALIGN(2))

/* Divisor used when deciding whether to split a free block.  A value of 1
 * means "always split", larger values raise the threshold. */
#define SPLIT_THRESHOLD_DIVIDER 1

/* ── Allocator API ────────────────────────────────────────────────────────── */

/* Allocate 'size' bytes from the heap and return a pointer to the payload.
 * Returns NULL if no suitable block is available. */
void *my_malloc(size_t size);

/* Release the block pointed to by 'ptr' back to the heap.
 * 'ptr' must be a value previously returned by my_malloc, or NULL (no-op). */
void my_free(void *ptr);

#endif /* HEAP_H */
