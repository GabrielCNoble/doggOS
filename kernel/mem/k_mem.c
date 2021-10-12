#include <stddef.h>
#include "k_mem.h"
#include "k_pstate.h"
#include "../k_term.h"
#include "../cpu/k_cpu.h"


// extern struct k_mem_range_t k_mem_low_range;
// extern uint32_t k_mem_range_count;
uint32_t k_mem_total = 0;
uint32_t k_mem_reserved = 0;
// extern struct k_mem_range_t k_mem_ranges[];

// extern uint32_t k_mem_gdt_desc_count;
// extern struct k_mem_seg_desc_t k_mem_gdt[]; 

// extern struct k_mem_pentry_t k_mem_pdirs[];
// struct k_mem_pstate_h k_mem_pstate;

// extern uint32_t k_kernel_start;
// extern uint32_t k_kernel_end;

// uint32_t k_mem_first_used_page_entry_index;
// uint32_t k_mem_last_used_page_entry_index;
// /* used pages list. The page address is used as an index into this list, which means each entry 
// refers to the same page. For entries of pages that are not in use, it refers to the entry in the 
// available pages list that specific page is located. For example, if the entry for the first page 
// contains an index of 4, that means the first page is located in the fifth entry of the available
// pages list. */
// struct k_mem_uppentry_t *k_mem_used_pages;

// /* available pages list. Each entry contains the physical address of the page. This list is likely
// not sorted most of the time. */
// uint32_t *k_mem_free_pages;
// /* number of free pages in the list */
// uint32_t k_mem_free_pages_count;
// /* maximum number of pages in the list, ever */
// uint32_t k_mem_free_pages_max_count;

struct k_mem_state_t k_mem_state;
uint32_t k_mem_range_count;
struct k_mem_range_t *k_mem_ranges;

void k_mem_Init(struct k_mem_range_t *ranges, uint32_t range_count)
{
    for(uint32_t range_index = 0; range_index < range_count; range_index++)
    {
        struct k_mem_range_t *range = ranges + range_index;

        uint32_t align = (uint32_t)range->base % 4096;

        if(align)
        {
            align = 4096 - align;
        }

        range->base += align;
        range->size -= align;
        k_mem_total += (uint32_t)range->size;
    }

    /* how many headers fit into a single page */
    uint32_t headers_per_page = 4096 / sizeof(struct k_mem_pentry_t);

    /* each header page will cover a block of 4096 * headers_per_page bytes, and
    for each such block we'll need to spend an extra 4096 bytes for the page that 
    covers it */
    uint32_t covered_bytes_block_size = 4096 * headers_per_page + 4096;
    /* this tells us how many of those covered byte blocks + header pages fit into
    the total free memory we have, which in turns gives us how many header pages we'll
    need to cover all the free memory, after subtracting away the amount spent for 
    those header pages */
    uint32_t free_page_headers = k_mem_total / covered_bytes_block_size;

    if(k_mem_total % covered_bytes_block_size)
    {
        free_page_headers++;
    }

    uint32_t free_page_header_bytes = free_page_headers * 4096;



    // uint32_t bits_per_page = 4096 * 8;
    // covered_bytes_block_size = 4096 * (bits_per_page / K_MEM_USED_PAGE_BITS) + 4096;
    // /* to simplify mapping page addresses to its proper byte/bit, we'll allocate
    // enough bytes to cover the whole 4GB address space */
    // uint32_t used_page_pages = 0xffffffff / covered_bytes_block_size;
    // if(0xffffffff % covered_bytes_block_size)
    // {
    //     used_page_pages++;
    // }
    // uint32_t used_page_bytes = used_page_pages * 4096;
    uint32_t used_page_entry_count = 0xffffffff / K_MEM_4KB_ADDRESS_OFFSET;
    uint32_t used_page_bytes = sizeof(struct k_mem_uppentry_t) * used_page_entry_count;
    
    uint32_t total_size = free_page_header_bytes + used_page_bytes;
    k_mem_reserved += total_size;

    struct k_mem_range_t *lowest_range = NULL;
    uint32_t lowest_range_index = 0;
    for(uint32_t range_index = 0; range_index < range_count; range_index++)
    {
        struct k_mem_range_t *range = ranges + range_index;
        if(range->size >= total_size)
        {
            if(!lowest_range || range->base < lowest_range->base)
            {
                lowest_range = range;
                lowest_range_index = range_index;
            }
        }
    }

    if(!lowest_range)
    {
        /* well, this would be a kernel panic, since there's not enough memory
        for some essential data structures */
    }

    k_mem_state.pheap.free_pages = (uint32_t *)((uint32_t)lowest_range->base);
    lowest_range->base += free_page_header_bytes;
    k_mem_state.pheap.used_pages = (struct k_mem_uppentry_t *)((uint32_t)lowest_range->base);
    lowest_range->base += used_page_bytes;

    uint32_t align = (uint32_t)lowest_range->base % 4096;

    if(align)
    {
        align = 4096 - align;
        lowest_range->base += align;
        total_size += align;
    }

    uint32_t data_end = lowest_range->base;

    if(lowest_range->size > total_size)
    {
        /* the range we used to store our data still has some space left */
        lowest_range->size -= total_size;

        if(lowest_range->size < K_MEM_4KB_ADDRESS_OFFSET)
        {
            /* range has less than 4KB available, which is not usable */
            ranges[lowest_range_index] = ranges[range_count];
            range_count--;
        }
    }
    else
    {
        /* the range was completely used, so drop it from the list */
        ranges[lowest_range_index] = ranges[range_count];
        range_count--;
    }

    for(uint32_t entry_index = 0; entry_index < used_page_entry_count; entry_index++)
    {
        /* we'll initialize all entries to an invalid entry index. Some physical addresses don't refer
        to any actual ram, so we need to mark them as such */
        k_mem_state.pheap.used_pages[entry_index].pid_index = K_MEM_USED_SMALL_PAGE_INVALID_ENTRY_INDEX;
        k_mem_state.pheap.used_pages[entry_index].flags = 0;
    }

    k_mem_state.pheap.free_pages_count = 0;
    k_mem_state.pheap.first_used_page = 0xffffffff;
    k_mem_state.pheap.last_used_page = 0;
    
    for(uint32_t range_index = 0; range_index < range_count; range_index++)
    {
        struct k_mem_range_t *range = ranges + range_index;
        uint32_t page_address = (uint32_t)range->base;
        uint32_t page_count = (uint32_t)range->size / K_MEM_4KB_ADDRESS_OFFSET;

        for(uint32_t page_index = 0; page_index < page_count; page_index++)
        {
            k_mem_state.pheap.free_pages[k_mem_state.pheap.free_pages_count] = page_address;
            uint32_t entry_index = K_MEM_USED_SMALL_PAGE_ENTRY_INDEX(page_address);
            struct k_mem_uppentry_t *used_entry = k_mem_state.pheap.used_pages + entry_index;
            used_entry->pid_index = k_mem_state.pheap.free_pages_count;
            used_entry->flags = 0;
            page_address += K_MEM_4KB_ADDRESS_OFFSET;

            if(entry_index < k_mem_state.pheap.first_used_page)
            {
                k_mem_state.pheap.first_used_page = entry_index;
            }

            if(entry_index > k_mem_state.pheap.last_used_page)
            {
                k_mem_state.pheap.last_used_page = entry_index;
            }

            k_mem_state.pheap.free_pages_count++;
        }
    }

    k_mem_state.pheap.free_pages_max_count = k_mem_state.pheap.free_pages_count;

    // k_printf("first page: %x - last page: %x\n", k_mem_free_pages[0], k_mem_free_pages[k_mem_free_pages_count - 1]);

    /* to simplify things further down the road we set up a really simple page state here. 
    All but the last page dir entry will map 4MB pages. The last entry will contain a page
    table, which will map the pages containing this page dir and this page table. This 'kinda'
    page state will be thrown away afterwards. */

    struct k_mem_pentry_t *page_dir = (struct k_mem_pentry_t *)k_mem_AllocPage(0);
    struct k_mem_pentry_t *page_table = (struct k_mem_pentry_t *)k_mem_AllocPage(0);
    struct k_mem_pentry_t *entry;
    /* map the page that contains the page directory */
    entry = page_table + K_MEM_PSTATE_PDIR_PTABLE_INDEX;
    entry->entry = ((uint32_t)page_dir) | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS;
    /* map the page that contains the page table */
    entry = page_table + K_MEM_PSTATE_PTABLE_PTABLE_INDEX;
    entry->entry = ((uint32_t)page_table) | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS; 
    /* point the last page directory entry to the page table we just set up */
    entry = page_dir + K_MEM_PSTATE_SELF_PDIR_INDEX;
    entry->entry = ((uint32_t)page_table) | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS;
    uint32_t address = 0;

    for(uint32_t entry_index = 0; entry_index < K_MEM_PSTATE_LAST_USABLE_PDIR_INDEX; entry_index++)
    {
        /* identity map the rest of the address space */
        struct k_mem_pentry_t *entry = page_dir + entry_index;
        entry->entry = address | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_BIG_PAGE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS;
    }

    k_cpu_Lcr3((uint32_t)page_dir);
    k_cpu_EnablePaging();

    /* we need to create the kernel pstate sort of manually here, because the "full" mechanism relies on the 
    heap manager to get an exclusive 4MB block, but right now there isn't a heap manager in any address space,
    so we use a fixed address that's guaranteed to not be in use right now. This mechanism could be made to 
    always use this fixed address, but that either would require a lot of synchronization, because more than one
    core might be trying to map a pstate while running kernel code, or several fixed 4MB blocks will need to be
    set aside (one for each core) to get rid of syncronization, which is not terrible, but not great either.  */


    /* we need to make sure the physical pages are pinned, otherwise they may get swaped out, 
    and that will cause all sorts of funny problems */
    k_mem_state.pstate.self_page = k_mem_AllocPage(K_MEM_PAGE_FLAG_PINNED);
    k_mem_state.pstate.pdir_page = k_mem_AllocPage(K_MEM_PAGE_FLAG_PINNED);
    k_mem_state.pstate.ptable_page = k_mem_AllocPage(K_MEM_PAGE_FLAG_PINNED);

    /* this will both map and initialize the page directory and page table for us */
    struct k_mem_pstate_t mapped_pstate;
    k_mem_MapPStateToAddress(&k_mem_state.pstate, K_MEM_ACTIVE_PSTATE_INIT_PSTATE_BLOCK_ADDRESS, &mapped_pstate);

    address = 0;

    while(address <= data_end)
    {
        k_mem_MapAddress(&mapped_pstate, address, address, K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS);
        address += K_MEM_4KB_ADDRESS_OFFSET;
    }
    k_mem_LoadPState(&k_mem_state.pstate); 

    // data_end = (data_end + 4093) & (~4093);
    // struct k_mem_bchunk_t *chunk = (struct k_mem_bchunk_t *)data_end;
    // uint32_t chunk_page = k_mem_AllocPage(0);
    // k_mem_MapAddress(&K_MEM_ACTIVE_MAPPED_PSTATE, chunk_page, (uint32_t)chunk, K_MEM_PENTRY_FLAG_READ_WRITE);

    // chunk->next = NULL;
    // chunk->prev = NULL;
    /* last usable virtual address before the stuff reserved for the kernel */
    // chunk->size = ((K_MEM_4MB_ADDRESS_OFFSET * K_MEM_PSTATE_LAST_USABLE_PDIR_INDEX) | 0x003ff000) - data_end;
    // k_mem_state.vheap.first_free_chunk = chunk;
    // k_mem_state.vheap.last_free_chunk = chunk;

    k_mem_ranges = ranges;
    k_mem_range_count = range_count;
    /* from now on, all the pstate creation and manipulation is available. Also, we purposefully won't unmap
    the kernel pstate here, because we used a fixed address */

    k_mem_FreePages((uint32_t)page_dir);
    k_mem_FreePages((uint32_t)page_table);
}
