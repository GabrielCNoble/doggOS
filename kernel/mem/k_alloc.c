#include "k_alloc.h"
#include "k_phys.h"
#include "k_pmap.h"
#include "../k_int.h"

uint16_t k_mem_small_bucket_sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048};

uint32_t k_mem_BigChunkIndexFromSize(uint32_t size)
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

void *k_mem_BigAlloc(struct k_mem_bheap_t *heap, uint32_t size)
{
    (void)heap;
    (void)size;
    return NULL;
}

void k_mem_BigReallocFree(struct k_mem_bheap_t *heap, void *memory, uint32_t size)
{
    (void)heap;
    (void)memory;
    (void)size;
}

void k_mem_BigFree(struct k_mem_bheap_t *heap, void *memory)
{
    (void)heap;
    (void)memory;
}

uint32_t k_mem_SmallBucketIndexFromSize(uint32_t size)
{
    for(uint32_t bucket_index = 0; bucket_index < K_MEM_SMALL_BUCKET_COUNT; bucket_index++)
    {
        if(size <= k_mem_small_bucket_sizes[bucket_index])
        {
            return bucket_index;
        }
    }

    return 0xffffffff;
}

void k_mem_InitChunkPageToBucket(uint32_t bucket_index)
{
    (void)bucket_index;
}

void *k_mem_SmallAlloc(struct k_mem_sheap_t *heap, uint32_t size)
{
    (void)heap;
    uint32_t bucket_index = k_mem_SmallBucketIndexFromSize(size);
    (void)bucket_index;
    void *memory = NULL;

    // if(bucket_index != 0xffffffff)
    // {
    //     struct k_mem_sbucket_t *bucket = k_mem_state.vheap.buckets + bucket_index;

    //     if(!bucket->first_chunk)
    //     {
    //         struct k_mem_scpage_t *new_page;
    //         struct k_mem_bchunk_t *best_chunk;
    //         struct k_mem_bchunk_t *big_chunk = k_mem_state.vheap.first_free_chunk;

    //         while(big_chunk)
    //         {
    //             if(!best_chunk || big_chunk->size < best_chunk->size)
    //             {
    //                 best_chunk = big_chunk;
    //             }

    //             big_chunk = big_chunk->next;
    //         }

    //         if(best_chunk->size == 1)
    //         {
    //             // k_mem_state.vheap.first_free_chunk = big_chunk->next;
    //         }
    //         else
    //         {
    //             // struct k_mem_bchunk_t *new_first_chunk = (struct k_mem_bchunk_t *)((uint32_t)big_chunk + 4096);
    //             // new_first_chunk->next = big_chunk->next;
    //             // new_first_chunk->size = big_chunk->size - 1;

    //             // if(big_chunk->next)
    //             // {
    //             //     big_chunk->next->prev = new_first_chunk;
    //             // }

    //             // k_mem_state.vheap.first_free_chunk = new_first_chunk;

    //             // if(big_chunk == k_mem_state.vheap.first_free_chunk)
    //             // {
    //             //     k_mem_state.vheap.last_free_chunk = new_first_chunk;
    //             // }
    //         }
    //     }
    //     else if(bucket->first_chunk == bucket->last_chunk)
    //     {
    //         bucket->last_chunk = NULL;
    //     }
        
    //     struct k_mem_schunk_t *chunk = bucket->first_chunk;
    //     bucket->first_chunk = bucket->first_chunk->next;

    //     chunk->next = NULL;
    //     chunk->prev = NULL;
    // }

    return memory;
}

void *k_mem_SmallRealloc(struct k_mem_sheap_t *heap, void *memory, uint32_t size)
{
    (void)heap;
    (void)memory;
    (void)size;
    return NULL;
}

void k_mem_SmallFree(struct k_mem_sheap_t *heap, void *memory)
{
    (void)heap;
    (void)memory;
}

void *k_mem_Malloc(struct k_mem_sheap_t *heap, uint32_t size, uint32_t align)
{
    (void)heap;
    (void)align;
    void *memory = NULL;

    if(size)
    {
        uint32_t page_count = size >> K_MEM_4KB_ADDRESS_SHIFT;

        if(!page_count)
        {
            page_count++;
        }

        uint32_t memory_pages = k_mem_AllocPages(page_count, 0);

        for(uint32_t page_index = 0; page_index < page_count; page_index++)
        {
            uint32_t page_address = memory_pages + page_index * K_MEM_4KB_ADDRESS_OFFSET;
            k_mem_MapAddress(page_address, page_address, K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS);
        }

        memory = (void *)memory_pages;
    }

    return memory;
}

void k_mem_Free(struct k_mem_sheap_t *heap, void *memory)
{
    (void)heap;
    if(memory)
    {
        uint32_t page_address = (uint32_t)memory;
        k_mem_FreePages(page_address);
    }
}