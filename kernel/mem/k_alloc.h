#ifndef K_ALLOC_H
#define K_ALLOC_H

#include "k_defs.h"
#include <stdalign.h>


void *k_mem_BigAlloc(struct k_mem_bheap_t *heap, uint32_t size);

void k_mem_BigReallocFree(struct k_mem_bheap_t *heap, void *memory, uint32_t size);

void k_mem_BigFree(struct k_mem_bheap_t *heap, void *memory);

uint32_t k_mem_SmallBucketIndexFromSize(uint32_t size);

void k_mem_InitChunkPage(struct k_mem_sheap_t *heap, uint32_t bucket_index);

void *k_mem_SmallAlloc(struct k_mem_sheap_t *heap, uint32_t size);

void *k_mem_SmallRealloc(struct k_mem_sheap_t *heap, void *memory, uint32_t size);

void k_mem_SmallFree(struct k_mem_sheap_t *heap, void *memory);

void *k_mem_Malloc(struct k_mem_sheap_t *heap, uint32_t size, uint32_t align);

void k_mem_Free(struct k_mem_sheap_t *heap, void *memory);

#endif