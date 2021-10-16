#ifndef DG_MALLOC_H
#define DG_MALLOC_H

#include <stdint.h>

void *dg_Malloc(uint32_t size, uint32_t align);

void dg_Free(void *memory);

#endif