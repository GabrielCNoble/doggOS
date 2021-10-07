#include "k_mem_vheap.h"
#include "../k_int.h"

uint16_t k_mem_bucket_sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048};

extern struct k_mem_state_t k_mem_state;

uint32_t k_mem_BucketIndexFromSize(uint32_t size)
{
    for(uint32_t bucket_index = 0; bucket_index < K_MEM_VHEAP_BUCKET_COUNT; bucket_index++)
    {
        if(size <= k_mem_bucket_sizes[bucket_index])
        {
            return bucket_index;
        }
    }

    return 0xffffffff;
}

void k_mem_InitChunkPageToBucket(uint32_t bucket_index)
{

}

void *k_mem_SmallAlloc(uint32_t size)
{
    uint32_t bucket_index = k_mem_BucketIndexFromSize(size);
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

void *k_mem_SmallRealloc(void *memory, uint32_t size)
{
    return NULL;
}

void k_mem_SmallFree(void *memory)
{

}

void *k_mem_Malloc(uint32_t size, uint32_t align)
{
    return k_mem_SmallAlloc(size);
}

void k_mem_Free(void *memory)
{
    k_mem_SmallFree(memory);
}

// void k_mem_AddBlock(uint32_t block_address, uint32_t block_size)
// {
//     (void)block_address;
//     (void)block_size;
//     // if(block_size >= sizeof(struct k_mem_block_t) + K_MEM_MIN_ALLOC_SIZE)
//     // {
//     //     block_size -= sizeof(struct k_mem_block_t);

//     //     k_mem_make_page_resident(block_address);

//     //     // if(!k_mem_is_address_mapped(K_MEM_ACTIVE_PSTATE, block_address))
//     //     // {
//     //     //     uint32_t page = k_mem_alloc_page();
//     //     //     k_mem_map_address(K_MEM_ACTIVE_PSTATE, page, block_address, K_MEM_PENTRY_FLAG_READ_WRITE);
//     //     // }

//     //     uint32_t next_page_address = (block_address & K_MEM_PENTRY_ADDR_MASK) + K_MEM_4KB_ADDRESS_OFFSET;

//     //     if(next_page_address - block_address < sizeof(struct k_mem_block_t))
//     //     {
//     //         k_mem_make_page_resident(next_page_address);
//     //     }

//     //     struct k_mem_block_t *block = (struct k_mem_block_t *)block_address;
//     //     block->size = block_size;
//     //     block->next = NULL;
//     //     block->prev = NULL;

//     //     k_mem_release_block((uint32_t)(block + 1));
//     // }
// }

// uint32_t k_mem_ReserveBlock(uint32_t size, uint32_t align)
// {
//     (void)size;
//     (void)align;
//     // struct k_mem_pstate_p *pstate = K_MEM_ACTIVE_PSTATE;
//     // struct k_mem_heapm_t *heap = &pstate->heapm;
//     // struct k_mem_block_t *block = heap->blocks; 

//     // if(!align)
//     // {
//     //     align = 1;
//     // }

//     // if(size < K_MEM_MIN_ALLOC_SIZE)
//     // {
//     //     size = K_MEM_MIN_ALLOC_SIZE;
//     // }

//     // while(block)
//     // {
//     //     uint32_t block_address = (uint32_t)(block + 1);
//     //     uint32_t address_align = block_address % align;
//     //     uint32_t block_size = block->size;

//     //     if(address_align)
//     //     {
//     //         address_align = align - address_align;
//     //         block_address += address_align;
//     //         block_size -= address_align;
//     //     }
        
//     //     if(block_size >= size)
//     //     {
//     //         if(block_size > size)
//     //         {
//     //             uint32_t new_block_address = block_address + size;
//     //             address_align = new_block_address % sizeof(struct k_mem_block_t );

//     //             if(address_align)
//     //             {
//     //                 address_align = sizeof(struct k_mem_block_t ) - address_align;
//     //                 new_block_address += address_align;
//     //                 size += address_align;
//     //             }

//     //             uint32_t new_block_size = block_size - size;

//     //             if(size < block_size && new_block_size >= sizeof(struct k_mem_block_t) + K_MEM_MIN_ALLOC_SIZE)
//     //             {
//     //                 k_mem_make_page_resident(new_block_address);

//     //                 struct k_mem_block_t *new_block = (struct k_mem_block_t *)new_block_address;
//     //                 new_block->size = new_block_size - sizeof(struct k_mem_block_t);
//     //                 new_block->next = block->next;
//     //                 if(new_block->next) 
//     //                 {
//     //                     new_block->next->prev = new_block;
//     //                 }

//     //                 new_block->prev = block;
//     //                 block->next = new_block;
//     //             }
//     //         }

//     //         block->size = size;

//     //         if(block->prev)
//     //         {
//     //             block->prev->next = block->next;
//     //         }
//     //         else
//     //         {
//     //             heap->blocks = block->next;
//     //         }

//     //         if(block->next)
//     //         {
//     //             block->next->prev = block->prev;
//     //         }
//     //         else
//     //         {
//     //             heap->last_block = block->prev;
//     //         }

//     //         block->next = NULL;
//     //         block->prev = NULL;

//     //         return block_address;
//     //     }

//     //     block = block->next;
//     // }

//     return 0;
// }

// void k_mem_ReleaseBlock(uint32_t address)
// {
//     (void)address;
//     // struct k_mem_pstate_p *pstate = K_MEM_ACTIVE_PSTATE;
//     // struct k_mem_heapm_t *heap = &pstate->heapm;
//     // struct k_mem_block_t *block = (struct k_mem_block_t *)address - 1;

//     // block->next = NULL;
//     // block->prev = NULL;

//     // if(!heap->blocks)
//     // {
//     //     heap->blocks = block;
//     // }
//     // else
//     // {
//     //     block->prev = heap->last_block;
//     //     heap->last_block->next = block;
//     // }

//     // heap->last_block = block;
//     // heap->block_count++;
// }

// void *k_mem_Alloc(uint32_t size, uint32_t align)
// {
//     // uint32_t block_address = k_mem_reserve_block(size, align);
//     // k_mem_make_alloc_resident((void *)block_address);
//     // return (void *)block_address;
//     (void)size;
//     (void)align;

//     return NULL;
// }

// void k_mem_MakeAllocResident(void *memory)
// {
//     (void)memory;
//     // if(memory)
//     // {
//     //     uint32_t alloc_address = (uint32_t)memory;
//     //     struct k_mem_block_t *block = (struct k_mem_block_t *)memory - 1;
//     //     k_mem_make_page_resident((uint32_t)block);

//     //     uint32_t first_page = alloc_address & K_MEM_SMALL_PAGE_ADDR_MASK;
//     //     uint32_t last_page = (alloc_address + block->size) & K_MEM_SMALL_PAGE_ADDR_MASK;

//     //     while(first_page != last_page)
//     //     {
//     //         k_mem_make_page_resident(first_page);
//     //         first_page += K_MEM_4KB_ADDRESS_OFFSET;
//     //     }
//     // }
// }

// void k_mem_Free(void *memory) 
// {
//     (void)memory;
//     // if(memory)
//     // {
//     //     struct k_mem_block_t *block = (struct k_mem_block_t *)memory - 1;


//     // }
// }