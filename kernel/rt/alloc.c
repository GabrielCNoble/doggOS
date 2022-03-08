#include "alloc.h"
#include "../mem/mngr.h"
#include "../mem/pmap.h"
#include "../defs.h"
#include "../k_int.h"
// #include "../k_term.h"
#include "../proc/thread.h"

uint16_t k_rt_small_bucket_sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048};

uint32_t k_rt_BigBucketIndexFromSize(size_t size)
{
    if(size == 1)
    {
        return 0;
    }

    size = (size >> K_RT_BCHUNK_BUCKET_SHIFT) + 1;

    if(size >= K_RT_BCHUNK_BUCKET_COUNT)
    {
        size = K_RT_BCHUNK_BUCKET_COUNT - 1;
    }

    return size;
}

void *k_rt_BigAlloc(size_t size, size_t align)
{
    (void)align;
    void *memory = NULL;

    if(size)
    {
        // k_atm_SpinLock(&heap->spinlock);
        
        size_t page_count = size >> K_MEM_4KB_ADDRESS_SHIFT;

        if(!page_count)
        {
            page_count++;
        }

        uintptr_t virtual_alloc = (uintptr_t )k_mem_AllocVirtualRange(size);
        uintptr_t virtual_page = virtual_alloc;
        uint32_t flags = K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS;
        for(size_t page_index = 0; page_index < page_count; page_index++)
        {
            // uintptr_t page_address = memory_pages + page_index * K_MEM_4KB_ADDRESS_OFFSET;
            uintptr_t page_address = k_mem_AllocPhysicalPage(0);
            if(k_mem_MapLinearAddress(virtual_page, page_address, flags) == K_STATUS_OUT_OF_PHYSICAL_MEM)
            {
                /* TODO: undo all the page mappings done by this loop if the system runs out of memory, to 
                avoid having a bogus mapping, and all remaining available physical pages spent on a failed
                allocation */
                // k_atm_SpinUnlock(&heap->spinlock);
                return NULL;
            }

            virtual_page += 0x1000;
        }

        memory = (void *)virtual_alloc;
        // k_printf("\rthread %x released the alloc lock             \n", k_proc_GetCurrentThread());
        // k_atm_SpinUnlock(&heap->spinlock);
    }

    return memory;
}

void k_rt_BigReallocFree(void *memory, size_t size)
{
    (void)memory;
    (void)size;
}

void k_rt_BigFree(void *memory)
{
    if(memory)
    {
        // k_atm_SpinLock(&heap->spinlock);
        uintptr_t physical_page = k_mem_LinearAddressPhysicalPage((uintptr_t)memory);
        k_mem_UnmapLinearAddress((uintptr_t)memory);
        k_mem_FreeVirtualRange(memory);
        k_mem_FreePhysicalPages(physical_page);
        // k_atm_SpinUnlock(&heap->spinlock);
    }
}

uint32_t k_rt_SmallBucketIndexFromSize(size_t size)
{
    for(size_t bucket_index = 0; bucket_index < K_RT_SMALL_BUCKET_COUNT; bucket_index++)
    {
        if(size <= k_rt_small_bucket_sizes[bucket_index])
        {
            return bucket_index;
        }
    }

    return 0xffffffff;
}

void k_rt_InitChunkPage(uint32_t bucket_index)
{
    struct k_rt_sheap_t *heap = k_rt_GetSmallHeap();

    size_t chunk_size = k_rt_small_bucket_sizes[bucket_index];
    size_t slots_per_chunk = chunk_size / k_rt_small_bucket_sizes[0];
    size_t cur_offset = K_RT_SCPAGE_CHUNK_COUNT;
    size_t chunk_count = K_RT_SCPAGE_CHUNK_BYTES / chunk_size;

    struct k_rt_sbucket_t *bucket = heap->buckets + bucket_index;
    struct k_rt_scpage_t *chunk_page = k_rt_BigAlloc(4096, 4096);

    chunk_page->header.main_bucket = bucket_index;
    struct k_rt_schunk_t *last_chunk = NULL;
    struct k_rt_schunk_t *next_chunk = NULL;

    for(size_t chunk_index = 0; chunk_index < chunk_count; chunk_index++)
    {
        cur_offset -= slots_per_chunk;
        struct k_rt_schunk_t *chunk = chunk_page->chunks + cur_offset;   
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

        struct k_rt_schunk_t *chunk = chunk_page->chunks + cur_offset;
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

void *k_rt_SmallAlloc(size_t size)
{
    void *memory = NULL;
    uint32_t bucket_index = k_rt_SmallBucketIndexFromSize(size);
    
    if(bucket_index != 0xffffffff)
    {
        struct k_rt_sheap_t *heap = k_rt_GetSmallHeap();
        struct k_rt_sbucket_t *bucket = heap->buckets + bucket_index;

        if(!bucket->first_chunk)
        {
            k_rt_InitChunkPage(bucket_index);
        }

        struct k_rt_schunk_t *chunk = bucket->first_chunk;
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

void *k_rt_SmallRealloc(void *memory, size_t size)
{
    (void)memory;
    (void)size;
    return NULL;
}

void k_rt_SmallFree(void *memory)
{
    struct k_rt_sheap_t *heap = k_rt_GetSmallHeap();
    uintptr_t chunk_address = (uintptr_t)memory;
    uintptr_t chunk_page_address = chunk_address & 0xfffff000;
    struct k_rt_schunk_t *chunk = (struct k_rt_schunk_t *)memory;
    struct k_rt_scpage_t *chunk_page = (struct k_rt_scpage_t *)(chunk_page_address);
    uint32_t bucket_index = k_rt_SmallBucketIndexFromSize(chunk_address - chunk_page_address);

    if(bucket_index > chunk_page->header.main_bucket)
    {
        bucket_index = chunk_page->header.main_bucket;
    }

    struct k_rt_sbucket_t *bucket = heap->buckets + bucket_index;

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

void *k_rt_Malloc(size_t size, size_t align)
{
    (void)align;

    void *memory = NULL;

    if(size)
    {
        if(size > 2048)
        {
            memory = k_rt_BigAlloc(size, align);
        }
        else
        {
            memory = k_rt_SmallAlloc(size);
        }
    }

    return memory;
}

void k_rt_Free(void *memory)
{
    uintptr_t memory_address = (uintptr_t)memory;

    if(memory_address & 0xfff)
    {
        k_rt_SmallFree(memory);
    }
    else
    {
        k_rt_BigFree(memory);
    }
}

struct k_rt_sheap_t *k_rt_GetSmallHeap()
{
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
    return &current_thread->heap;
}