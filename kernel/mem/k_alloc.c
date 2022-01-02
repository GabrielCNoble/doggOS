#include "k_alloc.h"
#include "k_phys.h"
#include "k_pmap.h"
#include "../k_int.h"
#include "../k_term.h"

uint16_t k_mem_small_bucket_sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048};

uint32_t k_mem_BigBucketIndexFromSize(size_t size)
{
    if(size == 1)
    {
        return 0;
    }

    size = (size >> K_MEM_BCHUNK_BUCKET_SHIFT) + 1;

    if(size >= K_MEM_BCHUNK_BUCKET_COUNT)
    {
        size = K_MEM_BCHUNK_BUCKET_COUNT - 1;
    }

    return size;
}

void *k_mem_BigAlloc(struct k_mem_bheap_t *heap, size_t size, size_t align)
{
    (void)heap;
    (void)align;
    void *memory = NULL;

    if(size)
    {
        size_t page_count = size >> K_MEM_4KB_ADDRESS_SHIFT;

        if(!page_count)
        {
            page_count++;
        }

        uintptr_t memory_pages = k_mem_AllocPages(page_count, 0);

        for(size_t page_index = 0; page_index < page_count; page_index++)
        {
            uintptr_t page_address = memory_pages + page_index * K_MEM_4KB_ADDRESS_OFFSET;
            k_mem_MapAddress(page_address, page_address, K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS);
        }

        memory = (void *)memory_pages;
    }

    return memory;
}

void k_mem_BigReallocFree(struct k_mem_bheap_t *heap, void *memory, size_t size)
{
    (void)heap;
    (void)memory;
    (void)size;
}

void k_mem_BigFree(struct k_mem_bheap_t *heap, void *memory)
{
    (void)heap;

    if(memory)
    {
        uintptr_t page_address = (uintptr_t)memory;
        k_mem_FreePages(page_address);
    }
}

uint32_t k_mem_SmallBucketIndexFromSize(size_t size)
{
    for(size_t bucket_index = 0; bucket_index < K_MEM_SMALL_BUCKET_COUNT; bucket_index++)
    {
        if(size <= k_mem_small_bucket_sizes[bucket_index])
        {
            return bucket_index;
        }
    }

    return 0xffffffff;
}

void k_mem_InitChunkPage(struct k_mem_sheap_t *heap, uint32_t bucket_index)
{
    size_t chunk_size = k_mem_small_bucket_sizes[bucket_index];
    size_t slots_per_chunk = chunk_size / k_mem_small_bucket_sizes[0];
    size_t cur_offset = K_MEM_SCPAGE_CHUNK_COUNT;
    size_t chunk_count = K_MEM_SCPAGE_CHUNK_BYTES / chunk_size;

    struct k_mem_sbucket_t *bucket = heap->buckets + bucket_index;
    struct k_mem_scpage_t *chunk_page = k_mem_BigAlloc(heap->big_heap, 4096, 4096);

    chunk_page->header.main_bucket = bucket_index;
    struct k_mem_schunk_t *last_chunk = NULL;
    struct k_mem_schunk_t *next_chunk = NULL;

    for(size_t chunk_index = 0; chunk_index < chunk_count; chunk_index++)
    {
        cur_offset -= slots_per_chunk;
        struct k_mem_schunk_t *chunk = chunk_page->chunks + cur_offset;   
        chunk->next = next_chunk;

        if(next_chunk)
        {
            next_chunk->prev = chunk;
        }
        else
        {
            last_chunk = chunk;
        }

        next_chunk = chunk;
    }

    if(bucket->last_chunk)
    {
        bucket->last_chunk->next = next_chunk;
        next_chunk->prev = bucket->last_chunk;
    }
    else
    {
        bucket->first_chunk = next_chunk;
    }

    bucket->last_chunk = last_chunk;

    while(cur_offset)
    {
        slots_per_chunk >>= 1;
        cur_offset -= slots_per_chunk;
        bucket_index--;
        bucket = heap->buckets + bucket_index;

        struct k_mem_schunk_t *chunk = chunk_page->chunks + cur_offset;
        chunk->next = NULL;

        if(bucket->last_chunk)
        {
            bucket->last_chunk->next = chunk;
            chunk->prev = bucket->last_chunk;
        }
        else
        {
            bucket->first_chunk = chunk;
        }

        bucket->last_chunk = chunk;
    }
}

void *k_mem_SmallAlloc(struct k_mem_sheap_t *heap, size_t size)
{
    void *memory = NULL;
    uint32_t bucket_index = k_mem_SmallBucketIndexFromSize(size);
    
    if(bucket_index != 0xffffffff)
    {
        struct k_mem_sbucket_t *bucket = heap->buckets + bucket_index;

        if(!bucket->first_chunk)
        {
            k_mem_InitChunkPage(heap, bucket_index);
        }

        struct k_mem_schunk_t *chunk = bucket->first_chunk;
        bucket->first_chunk = bucket->first_chunk->next;

        if(bucket->first_chunk)
        {
            bucket->first_chunk->prev = NULL;
        }
        else
        {
            bucket->last_chunk = NULL;
        }

        memory = (void *)chunk;
    }

    return memory;
}

void *k_mem_SmallRealloc(struct k_mem_sheap_t *heap, void *memory, size_t size)
{
    (void)heap;
    (void)memory;
    (void)size;
    return NULL;
}

void k_mem_SmallFree(struct k_mem_sheap_t *heap, void *memory)
{
    uintptr_t chunk_address = (uintptr_t)memory;
    uintptr_t chunk_page_address = chunk_address & 0xfffff000;
    struct k_mem_schunk_t *chunk = (struct k_mem_schunk_t *)memory;
    struct k_mem_scpage_t *chunk_page = (struct k_mem_scpage_t *)(chunk_page_address);
    uint32_t bucket_index = k_mem_SmallBucketIndexFromSize(chunk_address - chunk_page_address);

    if(bucket_index > chunk_page->header.main_bucket)
    {
        bucket_index = chunk_page->header.main_bucket;
    }

    struct k_mem_sbucket_t *bucket = heap->buckets + bucket_index;

    chunk->next = NULL;
    chunk->prev = NULL;

    chunk->next = bucket->first_chunk;
    bucket->first_chunk = chunk;

    if(chunk->next)
    {
        chunk->next->prev = chunk;
    }
    else
    {
        bucket->last_chunk = chunk;
    }
}

void *k_mem_Malloc(struct k_mem_sheap_t *heap, size_t size, size_t align)
{
    (void)align;

    void *memory = NULL;

    if(size)
    {
        if(size > 2048)
        {
            memory = k_mem_BigAlloc(heap->big_heap, size, align);
        }
        else
        {
            memory = k_mem_SmallAlloc(heap, size);
        }
    }

    return memory;
}

void k_mem_Free(struct k_mem_sheap_t *heap, void *memory)
{
    uintptr_t memory_address = (uintptr_t)memory;

    if(memory_address & 0xfff)
    {
        k_mem_SmallFree(heap, memory);
    }
    else
    {
        k_mem_BigFree(heap->big_heap, memory);
    }
}