#include <stddef.h>
#include "heap.h"

static char arr[PREALLOCATED_HEAP_SIZE] = {NULL};

size_t* heap_start(){
	return (size_t *)arr;
}

size_t* heap_end(){
	return (size_t *)(arr + PREALLOCATED_HEAP_SIZE);
}

void *find_fit(size_t size){
	size_t *header = heap_start();
	size_t *next;
	
	while(header < heap_end()){
		if(!(*header &1)){
			if(*header >= size){
				return header;	
			}
			else{
				next = (size_t *)((char *)header + *header);

				if(next < heap_end() && !(*next &1)){
					*header += *next;
					continue;
				}
			}
		}
		header = (size_t *)((char *)header + (*header & ~1L));
	}
	return NULL;
}

void malloc(size_t size){
	


}
