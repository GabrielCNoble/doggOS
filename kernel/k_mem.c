#include "k_mem.h"
#include "k_term.h"
#include <stddef.h>

extern uint32_t k_mem_range_count;
extern struct k_mem_range_t k_mem_ranges[];
extern uint32_t k_kernel_start;
extern uint32_t k_kernel_end;

struct k_mem_alloc_t *k_allocs = NULL;
struct k_mem_alloc_t *k_last_alloc = NULL;

void k_mem_init()
{
    for(uint32_t range_index = 0; range_index < k_mem_range_count; range_index++)
    {
        struct k_mem_range_t *range = k_mem_ranges + range_index;

        if(range->base == 0)
        {
            /* low memory, which is where we're loaded. This range has to be split,
            since the space the kernel is already occuping (with code and data)
            can't be freed, and so also can't be considered for allocation */
            struct k_mem_range_t *new_range = k_mem_ranges + k_mem_range_count;
            k_mem_range_count++;

            /* new range starts right after where the kernel ends */
            new_range->base = k_kernel_end;
            new_range->size = range->size - k_kernel_end;
            new_range->type = range->type;

            /* current range size is equal to where the kernel starts */
            range->size = k_kernel_start;

            break;
        }
    }

    for(uint32_t range_index = 0; range_index < k_mem_range_count; range_index++)
    {
        struct k_mem_range_t *range = k_mem_ranges + range_index;
        struct k_mem_alloc_t *alloc = (uint32_t *)range->base;

        alloc->size = range->size - sizeof(struct k_mem_alloc_t);
        alloc->next = NULL;
        alloc->prev = NULL;

        if(!k_allocs)
        {
            k_allocs = alloc;
        }
        else
        {
            k_last_alloc->next = alloc;
            alloc->prev = k_last_alloc;
        }

        k_last_alloc = alloc;
    }
}

void *k_mem_alloc(uint32_t size)
{
    struct k_mem_alloc_t *alloc = k_allocs;
    
    // size += 

    while(alloc)
    {
        if(alloc->size >= size)
        {
            uint8_t *alloc_mem = (uint8_t *)(alloc + 1);

            if(alloc->size > size)
            {
                struct k_mem_alloc_t *new_alloc = alloc_mem + size;
            }
        }
    }
}

void k_mem_free(void *mem)
{

}