#ifndef HEAP_H
#define HEAP_h

#include <stddef.h>

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT -1))
#define SIZE_T_SZ (ALIGN(sizeof(size_t)))
#define PREALLOCATED_HEAP_SIZE 16384
#define MIN_BLK_SZ (ALIGN(2))
#define SPLIT_THRESHOLD_DIVIDER 1

void* my_malloc(size_t size);
void free(void *ptr);

#endif
