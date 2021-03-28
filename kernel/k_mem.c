#include "k_mem.h"
#include "k_term.h"
#include <stddef.h>

extern uint32_t k_mem_range_count;
extern struct k_mem_range_t k_mem_ranges[];

extern uint32_t k_mem_gdt_desc_count;
extern struct k_mem_seg_desc_t k_mem_gdt[]; 

extern struct k_mem_pentry_t k_mem_pdirs[];
struct k_mem_pentry_t *k_mem_ptables;

uint32_t k_mem_total = 0;
extern uint32_t k_kernel_start;
extern uint32_t k_kernel_end;

struct k_mem_alloc_t *k_chunks = NULL;
struct k_mem_alloc_t *k_last_chunk = NULL;
uint32_t k_chunk_count = 0;

 


struct k_test_t
{
    struct k_test_t *next;
    uint32_t data;
};

void k_mem_init()
{
    for(uint32_t range_index = 0; range_index < k_mem_range_count; range_index++)
    {
        struct k_mem_range_t *range = k_mem_ranges + range_index;

        if(range->base == 0)
        {
            /* to avoid having to individually allocate space for global vars and the stack segment, we'll
            just reserve the first 640K for all of those things. */
            for(uint32_t copy_index = range_index + 1; range_index < k_mem_range_count; range_index++)
            {
                k_mem_ranges[copy_index - 1] = k_mem_ranges[copy_index];
            }

            k_mem_range_count--;
            range_index--;
        }
        else
        {
            uint32_t align = (uint32_t)range->base % 4096;
            if(align)
            {
                align = 4096 - align;
            }

            range->base += align;
            range->size -= align;
        }
    }

    struct k_mem_range_t *lowest_range = k_mem_ranges;
    uint32_t lowest_range_index = 0;
    for(uint32_t range_index = 1; range_index < k_mem_range_count; range_index++)
    {
        struct k_mem_range_t *range = k_mem_ranges + range_index;

        if(range->base < lowest_range->base && range->size >= 0x00400000)
        {
            lowest_range = range;
            lowest_range_index = range_index;
        }
    }


    k_mem_ptables = (struct k_mem_pentry_t *)((uint32_t)lowest_range->base);
    
    if(lowest_range->size > 0x00400000)
    {
        lowest_range->base += 0x00400000;
        lowest_range->size -= 0x00400000;
    }
    else
    {
        k_mem_ranges[lowest_range_index] = k_mem_ranges[k_mem_range_count];
        k_mem_range_count--;
    }

    for(uint32_t range_index = 0; range_index < k_mem_range_count; range_index++)
    {
        struct k_mem_range_t *range = k_mem_ranges + range_index;
        uint32_t chunk_address = (uint32_t)range->base;
        uint32_t align = chunk_address % sizeof(struct k_mem_alloc_t);

        if(align)
        {
            align = sizeof(struct k_mem_alloc_t) - align;
        }
        chunk_address += align;

        struct k_mem_alloc_t *chunk = (struct k_mem_alloc_t *)(chunk_address);

        chunk->size = range->size - (sizeof(struct k_mem_alloc_t) + align);
        chunk->next = NULL;
        chunk->prev = NULL;

        k_mem_total += chunk->size;

        if(!k_chunks)
        {
            k_chunks = chunk;
        }
        else
        {
            k_last_chunk->next = chunk;
            chunk->prev = k_last_chunk;
        }

        k_last_chunk = chunk;
        k_chunk_count++;
    }

    k_mem_page_mem(0x00000000, 0x00000000, 1);
    k_mem_enable_paging();
}

void *k_mem_alloc(uint32_t size)
{
    struct k_mem_alloc_t *chunk = k_chunks;
    struct k_mem_alloc_t *new_chunk = NULL;
    struct k_mem_alloc_t *new_next;
    struct k_mem_alloc_t *new_prev;
    void *memory = NULL;

    if(size)
    {
        uint32_t align = size % sizeof(struct k_mem_alloc_t );
        if(align)
        {
            align = sizeof(struct k_mem_alloc_t ) - align;
            size += align;
        }

        while(chunk)
        {
            if(chunk->size >= size)
            {
                uint32_t chunk_address = (uint32_t)(chunk + 1);


                /* we'll either return the current chunk, or we'll split it and return the first 
                split. The second case only happens if this chunk has enough space to also fit
                a new header */

                if(chunk->size > size + sizeof(struct k_mem_alloc_t))
                {
                    new_chunk = (struct k_mem_alloc_t *)(chunk_address + size);
                    new_chunk->prev = chunk->prev;
                    new_chunk->next = chunk->next;
                    new_next = new_chunk;
                    new_prev = new_chunk;

                    new_chunk->size = chunk->size - (size + sizeof(struct k_mem_alloc_t));
                    chunk->size = size;
                }
                else
                {
                    new_chunk = chunk;
                    new_next = chunk->next;
                    new_prev = chunk->prev;
                    k_chunk_count--;
                }

                if(new_chunk->prev)
                {
                    new_chunk->prev->next = new_next;
                }
                else
                {
                    k_chunks = new_next;
                }

                if(new_chunk->next)
                {
                    new_chunk->next->prev = new_prev;
                }
                else
                {
                    k_last_chunk = new_prev;
                }

                /* skip header */
                memory = chunk + 1;
                break;
            }

            chunk = chunk->next;
        }
    }

    return memory;
}

void *k_mem_realloc(void *memory, uint32_t new_size)
{

}

void k_mem_free(void *mem)
{
    if(mem)
    {
        struct k_mem_alloc_t *chunk = (struct k_mem_alloc_t *)mem - 1;

        chunk->next = NULL;

        if(!k_chunks)
        {
            k_chunks = chunk;
        }
        else
        {
            k_last_chunk->next = chunk;
            chunk->prev = k_last_chunk;
        }

        k_last_chunk = chunk;
        k_chunk_count++;
    }
}

uint32_t k_mem_page_mem(uint32_t physical_address, uint32_t linear_address, uint32_t big_page)
{
    struct k_mem_pentry_t *page_entry = k_mem_pdirs + (linear_address >> 22);

    if(page_entry->entry & K_MEM_PENTRY_FLAG_USED)
    {
        return 0;
    }

    if(big_page)
    {
        page_entry->entry = physical_address & K_MEM_BIG_PAGE_PHYS_ADDR_MASK;
        page_entry->entry |= K_MEM_PENTRY_FLAG_BIG_PAGE | K_MEM_PENTRY_FLAG_PRESENT;
        page_entry->entry |= K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE;

        return 1;
    }

    return 0;
}