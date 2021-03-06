#include "k_mem.h"
#include "k_term.h"

extern uint32_t k_mem_range_count;
extern struct k_mem_range_t k_mem_ranges[];

void k_mem_init()
{
    k_puts("initializing memory... \n");
    for(uint32_t range_index = 0; range_index < k_mem_range_count; range_index++)
    {
        struct k_mem_range_t *range = k_mem_ranges + range_index;
        k_printf("mem range at 0x%llx, with size 0x%llx (%d) bytes available!\n", range->base, range->size, range->size);
    }
    k_puts("memory initialized!\n");
}