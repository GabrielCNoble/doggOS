#ifndef K_ALLOC_H
#define K_ALLOC_H

#include "k_defs.h"
#include <stdalign.h>

uint32_t k_mem_BigBucketIndexFromSize(size_t size);

void *k_mem_BigAlloc(struct k_mem_bheap_t *heap, size_t size, size_t align);

void k_mem_BigReallocFree(struct k_mem_bheap_t *heap, void *memory, size_t size);

void k_mem_BigFree(struct k_mem_bheap_t *heap, void *memory);

uint32_t k_mem_SmallBucketIndexFromSize(size_t size);

void k_mem_InitChunkPage(struct k_mem_sheap_t *heap, size_t bucket_index);

void *k_mem_SmallAlloc(struct k_mem_sheap_t *heap, size_t size);

void *k_mem_SmallRealloc(struct k_mem_sheap_t *heap, void *memory, size_t size);

void k_mem_SmallFree(struct k_mem_sheap_t *heap, void *memory);

void *k_mem_Malloc(struct k_mem_sheap_t *heap, size_t size, size_t align);

void k_mem_Free(struct k_mem_sheap_t *heap, void *memory);

#endif