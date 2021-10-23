#ifndef DG_MALLOC_H
#define DG_MALLOC_H

#include <stdint.h>

/*
====================
dg_Malloc: 
    
    returns a new block of memory with at least [size] bytes, aligned at [align] byte boundary.
    [align] has to be a power of two, and has a minimum value of 8. If a value smaller than 8 is 
    passed, the function will consider [align] as being 8. If a non-power of two value is passed,
    it'll be rounded up to the next power of two.

    Returns NULL on failure or when [size] is 0.
====================
*/
void *dg_Malloc(uint32_t size, uint32_t align);

/*
====================
dg_Free: 
    
    frees a block of memory pointed by [memory] previously returned by one of the allocation functions.
    passing NULL for the value of [memory] is a nop.
====================
*/
void dg_Free(void *memory);

#endif