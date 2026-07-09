/*
 * heap.h — Public interface for dumalloc.
 */
#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define SIZE_T_SZ (ALIGN(sizeof(size_t)))

#define PREALLOCATED_HEAP_SIZE 16384
#define MIN_BLK_SZ (SIZE_T_SZ + ALIGNMENT)
#define SPLIT_THRESHOLD_DIVIDER 1

/* Initialise the arena. Called automatically by my_malloc on first use. */
void heap_init(void);

void *my_malloc(size_t size);
void my_free(void *ptr);

#endif /* HEAP_H */
