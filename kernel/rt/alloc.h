#ifndef ALLOC_H
#define ALLOC_H

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include "atm.h"

#define K_RT_BCHUNK_BUCKET_COUNT 1024
#define K_RT_BCHUNK_BUCKET_SHIFT 10

struct k_rt_bchunk_t
{
    union
    {
        uint8_t bytes[4096];

        struct
        {
            struct k_rt_bchunk_t *next;
            struct k_rt_bchunk_t *prev;
            uint32_t size;
            uint32_t stack_top;
            struct k_rt_bchunk_t *chunk_stack[];
        };
    };
};

struct k_rt_bbucket_t
{
    struct k_rt_bchunk_t *out_chunks;
    struct k_rt_bchunk_t *in_chunks;
};

struct k_rt_bcheader_t
{
    uint32_t type : 2;
    uint32_t size : 30;
};

struct k_rt_bcpage_t
{
    struct k_rt_bcheader_t headers[1024];
};
struct k_rt_bheap_t
{
    struct k_rt_bcpage_t *cpages;
    struct k_rt_bbucket_t *buckets;
    k_rt_spnl_t spinlock;
};



#define K_RT_SMALL_BUCKET_COUNT 9
struct k_rt_schunk_t
{
    struct k_rt_schunk_t *next;
    struct k_rt_schunk_t *prev;
};

struct k_rt_scpheader_t
{
    struct k_rt_scpage_t *next_free;
    uint32_t main_bucket : 4;
    uint32_t free_count : 12;
};

#define K_RT_SCPAGE_CHUNK_BYTES (4096 - sizeof(struct k_rt_scpheader_t))
#define K_RT_SCPAGE_CHUNK_COUNT (K_RT_SCPAGE_CHUNK_BYTES / sizeof(struct k_rt_schunk_t))

struct k_rt_scpage_t
{
    struct k_rt_scpheader_t header;
    struct k_rt_schunk_t chunks[K_RT_SCPAGE_CHUNK_COUNT];
};

struct k_rt_sbucket_t
{
    struct k_rt_schunk_t *first_chunk;
    struct k_rt_schunk_t *last_chunk;
};

struct k_rt_sheap_t
{
    struct k_rt_bheap_t *big_heap;
    struct k_rt_sbucket_t buckets[K_RT_SMALL_BUCKET_COUNT];
    struct k_rt_scpage_t *free_pages;
    k_rt_spnl_t spinlock;
};

uint32_t k_rt_BigBucketIndexFromSize(size_t size);

void *k_rt_BigAlloc(size_t size, size_t align);

void k_rt_BigReallocFree(void *memory, size_t size);

void k_rt_BigFree(void *memory);

uint32_t k_rt_SmallBucketIndexFromSize(size_t size);

void k_rt_InitChunkPage(size_t bucket_index);

void *k_rt_SmallAlloc(size_t size);

void *k_rt_SmallRealloc(void *memory, size_t size);

void k_rt_SmallFree(void *memory);

void *k_rt_Malloc(size_t size, size_t align);

void k_rt_Free(void *memory);

struct k_rt_sheap_t *k_rt_GetSmallHeap();

#endif