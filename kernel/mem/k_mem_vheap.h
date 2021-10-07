#ifndef K_MEM_VHEAP_H
#define K_MEM_VHEAP_H

#include "k_mem_com.h"
#include <stdalign.h>

uint32_t k_mem_BucketIndexFromSize(uint32_t size);

void k_mem_InitChunkPageToBucket(uint32_t bucket_index);

void *k_mem_SmallAlloc(uint32_t size);

void *k_mem_SmallRealloc(void *memory, uint32_t size);

void k_mem_SmallFree(void *memory);

// void *k_mem_AllocPage(uint32_t zero_page);

void *k_mem_Malloc(uint32_t size, uint32_t align);

void k_mem_Free(void *memory);

// void *k_mem_Alloc(uint32_t size, uint32_t align);
// void k_mem_CreateChunk(uint32_t chunk_address, )

// void k_mem_AddBlock(uint32_t block_address, uint32_t block_size);

// uint32_t k_mem_ReserveBlock(uint32_t size, uint32_t align);

// void k_mem_ReleaseBlock(uint32_t address);

// void *k_mem_Alloc(uint32_t size, uint32_t align);

// void *k_mem_AllocC(uint32_t size, uint32_t align);

// void k_mem_MakeAllocResident(void *memory);

// void k_mem_MakeAllocResidentC(void *memory);

// void k_mem_Defrag();

// void k_mem_Free(void *memory);  

#endif