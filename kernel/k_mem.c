#include <stddef.h>
#include "k_mem.h"
#include "k_term.h"


extern struct k_mem_range_t k_mem_low_range;
extern uint32_t k_mem_range_count;
uint32_t k_mem_total = 0;
uint32_t k_mem_reserved = 0;
extern struct k_mem_range_t k_mem_ranges[];

extern uint32_t k_mem_gdt_desc_count;
extern struct k_mem_seg_desc_t k_mem_gdt[]; 

// extern struct k_mem_pentry_t k_mem_pdirs[];
struct k_mem_pstate_h k_mem_pstate;

extern uint32_t k_kernel_start;
extern uint32_t k_kernel_end;

uint32_t k_mem_first_used_page_entry_index;
uint32_t k_mem_last_used_page_entry_index;
/* used pages list. The page address is used as an index into this list, which means each entry 
refers to the same page. For entries of pages that are not in use, it refers to the entry in the 
available pages list that specific page is located. For example, if the entry for the first page 
contains an index of 4, that means the first page is located in the fifth entry of the available
pages list. */
struct k_mem_uppentry_t *k_mem_used_pages;

/* available pages list. Each entry contains the physical address of the page. This list is likely
not sorted most of the time. */
uint32_t *k_mem_free_pages;
/* number of free pages in the list */
uint32_t k_mem_free_pages_count;
/* maximum number of pages in the list, ever */
uint32_t k_mem_free_pages_max_count;

void k_mem_Init()
{
    for(uint32_t range_index = 0; range_index < k_mem_range_count; range_index++)
    {
        struct k_mem_range_t *range = k_mem_ranges + range_index;

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
    for(uint32_t range_index = 0; range_index < k_mem_range_count; range_index++)
    {
        struct k_mem_range_t *range = k_mem_ranges + range_index;
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

    k_mem_free_pages = (uint32_t *)((uint32_t)lowest_range->base);
    lowest_range->base += free_page_header_bytes;
    k_mem_used_pages = (struct k_mem_uppentry_t *)((uint32_t)lowest_range->base);
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
            k_mem_ranges[lowest_range_index] = k_mem_ranges[k_mem_range_count];
            k_mem_range_count--;
        }
    }
    else
    {
        /* the range was completely used, so drop it from the list */
        k_mem_ranges[lowest_range_index] = k_mem_ranges[k_mem_range_count];
        k_mem_range_count--;
    }

    for(uint32_t entry_index = 0; entry_index < used_page_entry_count; entry_index++)
    {
        /* we'll initialize all entries to an invalid entry index. Some physical addresses don't refer
        to any actual ram, so we need to mark them as such */
        k_mem_used_pages[entry_index].pid_index = K_MEM_USED_SMALL_PAGE_INVALID_ENTRY_INDEX;
        k_mem_used_pages[entry_index].flags = 0;
    }

    k_mem_free_pages_count = 0;
    k_mem_first_used_page_entry_index = 0xffffffff;
    k_mem_last_used_page_entry_index = 0;
    
    for(uint32_t range_index = 0; range_index < k_mem_range_count; range_index++)
    {
        struct k_mem_range_t *range = k_mem_ranges + range_index;
        uint32_t page_address = (uint32_t)range->base;
        uint32_t page_count = (uint32_t)range->size / K_MEM_4KB_ADDRESS_OFFSET;

        for(uint32_t page_index = 0; page_index < page_count; page_index++)
        {
            k_mem_free_pages[k_mem_free_pages_count] = page_address;
            uint32_t entry_index = K_MEM_USED_SMALL_PAGE_ENTRY_INDEX(page_address);
            struct k_mem_uppentry_t *used_entry = k_mem_used_pages + entry_index;
            used_entry->pid_index = k_mem_free_pages_count;
            used_entry->flags = 0;
            page_address += K_MEM_4KB_ADDRESS_OFFSET;

            if(entry_index < k_mem_first_used_page_entry_index)
            {
                k_mem_first_used_page_entry_index = entry_index;
            }

            if(entry_index > k_mem_last_used_page_entry_index)
            {
                k_mem_last_used_page_entry_index = entry_index;
            }

            k_mem_free_pages_count++;
        }
    }

    k_mem_free_pages_max_count = k_mem_free_pages_count;

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
    entry->entry = ((uint32_t)page_dir) | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_READ_WRITE;
    /* map the page that contains the page table */
    entry = page_table + K_MEM_PSTATE_PTABLE_PTABLE_INDEX;
    entry->entry = ((uint32_t)page_table) | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_READ_WRITE; 
    /* point the last page directory entry to the page table we just set up */
    entry = page_dir + K_MEM_PSTATE_SELF_PDIR_INDEX;
    entry->entry = ((uint32_t)page_table) | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_READ_WRITE;
    uint32_t address = 0;

    for(uint32_t entry_index = 0; entry_index < K_MEM_PSTATE_LAST_USABLE_PDIR_INDEX; entry_index++)
    {
        /* identity map the rest of the address space */
        struct k_mem_pentry_t *entry = page_dir + entry_index;
        entry->entry = address | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_BIG_PAGE;
    }

    k_mem_LoadPageDir((uint32_t)page_dir);
    k_mem_EnablePaging();

    /* we need to create the kernel pstate sort of manually here, because the "full" mechanism relies on the 
    heap manager to get an exclusive 4MB block, but right now there isn't a heap manager in any address space,
    so we use a fixed address that's guaranteed to not be in use right now. This mechanism could be made to 
    always use this fixed address, but that either would require a lot of synchronization, because more than one
    core might be trying to map a pstate while running kernel code, or several fixed 4MB blocks will need to be
    set aside (one for each core) to get rid of syncronization, which is not terrible, but not great either.  */


    /* we need to make sure the physical pages are pinned, otherwise they may get swaped out, 
    and that will cause all sorts of funny problems */
    k_mem_pstate.self_page = k_mem_AllocPage(K_MEM_PAGE_FLAG_PINNED);
    k_mem_pstate.pdir_page = k_mem_AllocPage(K_MEM_PAGE_FLAG_PINNED);
    k_mem_pstate.ptable_page = k_mem_AllocPage(K_MEM_PAGE_FLAG_PINNED);

    /* this will both map and initialize the page directory and page table for us */
    struct k_mem_pstate_t mapped_pstate;
    k_mem_MapPStateToAddress(&k_mem_pstate, K_MEM_ACTIVE_PSTATE_INIT_PSTATE_BLOCK_ADDRESS, &mapped_pstate);

    address = 0;

    while(address < data_end)
    {
        k_mem_MapAddress(&mapped_pstate, address, address, K_MEM_PENTRY_FLAG_READ_WRITE);
        address += K_MEM_4KB_ADDRESS_OFFSET;
    }

    k_mem_LoadPState(&k_mem_pstate);

    // for(uint32_t range_index = 0; range_index < k_mem_range_count; range_index++)
    // {
    //     struct k_mem_range_t *range = k_mem_ranges + range_index;
    //     k_mem_add_block(range->base, range->size);
    // }

    /* from now on, all the pstate creation and manipulation is available. Also, we purposefully won't unmap
    the kernel pstate here, because we used a fixed address */

    k_mem_FreePages((uint32_t)page_dir);
    k_mem_FreePages((uint32_t)page_table);
}

/*===========================================================================*/
/*===========================================================================*/
/*===========================================================================*/

void k_mem_SortFreePages()
{
    uint32_t first_free_entry_index = 0;

    for(uint32_t entry_index = k_mem_first_used_page_entry_index; entry_index <= k_mem_last_used_page_entry_index; entry_index++)
    {
        /* entry in the used pages list that refers to the current page */
        struct k_mem_uppentry_t *used_entry = k_mem_used_pages + entry_index;

        if(used_entry->flags & K_MEM_PAGE_FLAG_USED || used_entry->pid_index == K_MEM_USED_SMALL_PAGE_INVALID_ENTRY_INDEX)
        {
            /* this page is in use, which means it's not present in the list, or this address doesn't represent actual ram */
            continue;
        }

        uint32_t page_address = entry_index << K_MEM_4KB_ADDRESS_SHIFT;
        used_entry->pid_index = first_free_entry_index;
        k_mem_free_pages[first_free_entry_index] = page_address;
        first_free_entry_index++;

        // if(first_used_entry->pid_index != first_free_entry_index)
        // {
        //     /* this page is not in the right place in the free pages list, so we'll need
        //     to swap it with whatever is located there */

        //     uint32_t first_page_address = k_mem_free_pages[first_used_entry->pid_index];
        //     uint32_t second_page_address = k_mem_free_pages[first_free_entry_index];
        //     struct k_mem_uppentry_t *second_used_entry = k_mem_used_pages + K_MEM_USED_SMALL_PAGE_ENTRY_INDEX(second_page_address);

        //     k_mem_free_pages[first_used_entry->pid_index] = second_page_address;
        //     k_mem_free_pages[first_free_entry_index] = first_page_address;

        //     second_used_entry->pid_index = first_used_entry->pid_index;
        //     first_used_entry->pid_index = first_free_entry_index;
        // }
    }
}

uint32_t k_mem_AllocPage(uint32_t flags)
{
    uint32_t page_address = K_MEM_INVALID_PAGE;

    if(k_mem_free_pages_count)
    {
        k_mem_free_pages_count--;
        page_address = k_mem_free_pages[k_mem_free_pages_count];
        struct k_mem_uppentry_t *used_entry = k_mem_used_pages + K_MEM_USED_SMALL_PAGE_ENTRY_INDEX(page_address);
        used_entry->pid_index = 0;
        used_entry->flags = flags | K_MEM_PAGE_FLAG_USED | K_MEM_PAGE_FLAG_HEAD;
    }

    return page_address;
}

uint32_t k_mem_AllocPages(uint32_t page_count, uint32_t flags)
{
    uint32_t page_address = K_MEM_INVALID_PAGE;
    uint32_t entries_sorted = 0;

    if(k_mem_free_pages_count && k_mem_free_pages_count >= page_count)
    {
        _try_again:

        /* how many times we'll need to run the loop below if we take into consideration the number of
        pages we want to allocate. For example, imagine we want to allocate 4 pages and there are only
        6 available pages. There's no point in checking pages after the second page, because there won't 
        be enough pages to service the request anyway  */
        uint32_t entry_count = k_mem_free_pages_count - page_count;

        for(uint32_t entry_index = 0; entry_index <= entry_count; entry_index++)
        {
            uint32_t *head_entry = k_mem_free_pages + entry_index;
            uint32_t first_entry = head_entry[0];

            for(uint32_t next_entry_index = 1; next_entry_index < page_count; next_entry_index++)
            {
                uint32_t second_entry = head_entry[next_entry_index];
                /* likely not the ideal thing to do here. Could be more efficient to try
                both combinations, and swap the entries if those point to adjacent pages */
                if(first_entry > second_entry || second_entry - first_entry > K_MEM_4KB_ADDRESS_OFFSET)
                {
                    head_entry = NULL;
                    entry_index += next_entry_index;
                    break; 
                }

                first_entry = second_entry;
            }

            if(head_entry)
            {
                page_address = head_entry[0];
                struct k_mem_uppentry_t *used_head_entry = k_mem_used_pages + K_MEM_USED_SMALL_PAGE_ENTRY_INDEX(page_address);
                uint32_t first_page_index = used_head_entry->pid_index;
                uint32_t next_free_page_index = first_page_index + page_count;

                if(next_free_page_index > k_mem_free_pages_count)
                {
                    /* error: the allocator somehow "invented" extra pages */
                }

                uint32_t move_count = k_mem_free_pages_count - next_free_page_index;
                uint32_t move_offset = next_free_page_index - first_page_index;

                /* starts at the first page we're allocating */
                uint32_t *dst_pages = k_mem_free_pages + first_page_index;
                /* starts at the first page after the last page we're allocating */
                uint32_t *src_pages = k_mem_free_pages + next_free_page_index;

                for(uint32_t move_index = 0; move_index < move_count; move_index++)
                {
                    /* we'll move pages from the end of the list to fill the "hole", preserving their order. This is "slow",
                    but allows us to get away with not sorting pages more often */

                    /* we'll move a page into this entry, so offset the used page entry of this page by the amount of positions we'll move it */
                    struct k_mem_uppentry_t *src_page_used_entry = k_mem_used_pages + K_MEM_USED_SMALL_PAGE_ENTRY_INDEX(src_pages[move_index]);
                    src_page_used_entry->pid_index -= move_offset;

                    /* do the movzzzzzzzzzzzzzzzzzz... */
                    dst_pages[move_index] = src_pages[move_index];
                }
                k_mem_free_pages_count -= page_count;

                flags = (flags & ~K_MEM_PAGE_FLAG_HEAD) | K_MEM_PAGE_FLAG_USED;

                if(page_count > 1)
                {
                    /* pages in a contiguous allocation must be pinned, otherwise, they may get swapped out. That will likely cause
                    problems because if a non-head page gets swapped and reused as a head page, deallocation won't function properly.
                    This could be solved by swapping the page flags as well */
                    flags |= K_MEM_PAGE_FLAG_PINNED;
                }

                used_head_entry[0].pid_index = 0;
                used_head_entry[0].flags = flags | K_MEM_PAGE_FLAG_HEAD;

                for(uint32_t entry_index = 1; entry_index < page_count; entry_index++)
                {
                    /* at first, the kernel process owns those pages. The pid value will be later assigned
                    by the process allocating the page */
                    used_head_entry[entry_index].pid_index = 0;
                    used_head_entry[entry_index].flags = flags;
                }

                break;
            }
        }

        if(page_address == K_MEM_INVALID_PAGE && !entries_sorted)
        {
            k_mem_SortFreePages();
            entries_sorted = 1;
            goto _try_again;
        }
    }

    return page_address;
}

uint32_t k_mem_IsValidPage(uint32_t page_address)
{
    if(page_address != K_MEM_INVALID_PAGE)
    {
        return k_mem_used_pages[K_MEM_USED_SMALL_PAGE_ENTRY_INDEX(page_address)].pid_index != K_MEM_USED_SMALL_PAGE_INVALID_ENTRY_INDEX;
    }
}

void k_mem_FreePages(uint32_t page_address)
{
    page_address &= K_MEM_4KB_ADDRESS_MASK;

    if(page_address != K_MEM_INVALID_PAGE)
    {
        uint32_t entry_index = K_MEM_USED_SMALL_PAGE_ENTRY_INDEX(page_address);
        struct k_mem_uppentry_t *used_entry = k_mem_used_pages + entry_index;

        if((used_entry->flags & (K_MEM_PAGE_FLAG_USED | K_MEM_PAGE_FLAG_HEAD)) == (K_MEM_PAGE_FLAG_USED | K_MEM_PAGE_FLAG_HEAD))
        {
            do
            {
                used_entry->pid_index = k_mem_free_pages_count;
                used_entry->flags = 0;

                // k_printf("free page %x to %d\n", page_address, k_mem_free_pages_count);

                k_mem_free_pages[k_mem_free_pages_count] = page_address;
                k_mem_free_pages_count++;
                page_address += K_MEM_4KB_ADDRESS_OFFSET;
                entry_index++;

                used_entry = k_mem_used_pages + entry_index;
            }
            while(entry_index <= k_mem_last_used_page_entry_index &&
                ((used_entry->flags & (K_MEM_PAGE_FLAG_HEAD | K_MEM_PAGE_FLAG_USED)) == K_MEM_PAGE_FLAG_USED));
        }

        // if((k_mem_used_pages[byte_index] & valid_bits) == valid_bits)
        // {
        //     do
        //     {
        //         /* when consecutive pages are allocated, the first page gets flagged as the head page while 
        //         the rest allocated alongside aren't. To free those pages we first free the head page, and 
        //         then keep freeing consecutive pages until we find a page that has the head flag set. When 
        //         this happens, it means we reached another allocation and should stop. */
        //         k_mem_used_pages[byte_index] &= ~valid_bits;
        //         k_mem_avail_pages_cursor++;
        //         struct k_mem_pentry_t *entry = k_mem_avail_pages + k_mem_avail_pages_cursor;
        //         entry->entry = page_address; 
        //         page_address += K_MEM_4KB_ADDRESS_OFFSET;
        //         byte_index = K_MEM_SMALL_PAGE_USED_BYTE_INDEX(page_address);
        //         bit_index = K_MEM_SMALL_PAGE_USED_BIT_INDEX(page_address);
        //         valid_bits = (K_MEM_PAGE_FLAG_USED | K_MEM_PAGE_FLAG_HEAD) << bit_index;
        //     }
        //     while((k_mem_used_pages[byte_index] & valid_bits) == (uint8_t)(K_MEM_PAGE_FLAG_USED << bit_index));
        // }
        // else
        // {
        //     if((k_mem_used_pages[byte_index] & valid_bits) == (uint8_t)(K_MEM_PAGE_FLAG_USED << bit_index))
        //     {
        //         k_printf("attempt to free non-head page %x\n", page_address);
        //     }
        //     else
        //     {
        //         k_printf("attempt to free non-allocated page %x\n", page_address);
        //     }
        // }
    }
}

/*===========================================================================*/
/*===========================================================================*/
/*===========================================================================*/
struct k_mem_pstate_h k_mem_CreatePState()
{
    struct k_mem_pstate_h pstate = {};

    pstate.self_page = k_mem_AllocPage(K_MEM_PAGE_FLAG_PINNED);
    pstate.pdir_page = k_mem_AllocPage(K_MEM_PAGE_FLAG_PINNED);
    pstate.ptable_page = k_mem_AllocPage(K_MEM_PAGE_FLAG_PINNED);
    // pstate.block_table_page = k_mem_alloc_page(K_MEM_PAGE_FLAG_PINNED);

    struct k_mem_pstate_t mapped_pstate;
    /* this will initialize this pstate page directory and page table accordingly. We'll map it to a reserved
    position, destined for the exact purpose of initializing a pstate. */

    /* TODO: this WILL require synchronization */
    k_mem_MapPStateToAddress(&pstate, K_MEM_ACTIVE_PSTATE_INIT_PSTATE_BLOCK_ADDRESS, &mapped_pstate);

    return pstate;
}

void k_mem_MapPState(struct k_mem_pstate_h *pstate, struct k_mem_pstate_t *mapped_pstate)
{
    /* 4MB aligned block, so we can be sure it'll be contained in a single page
    directory. We're only reserving a portion of the linear memory space here, no
    physical memory is being commited */
    uint32_t map_address = k_mem_reserve_block(0x00400000, 0x00400000);
    k_mem_MapPStateToAddress(pstate, map_address, mapped_pstate);
}

void k_mem_MapPStateToAddress(struct k_mem_pstate_h *pstate, uint32_t map_address, struct k_mem_pstate_t *mapped_pstate)
{
    uint32_t dir_index = K_MEM_PDIR_INDEX(map_address);
    // struct k_mem_pstate_p *mapped_pstate = NULL;

    if(!(map_address & 0x003fffff))
    {
        /* only map this pstate if the address is aligned to a 4MB boundary */


        /* page dir entry in the active pstate that will use the page table of the pstate we want to map */
        struct k_mem_pentry_t *active_pdir_entry = K_MEM_ACTIVE_PSTATE_PDIR_MAP_POINTER + dir_index;

        /* entry in the active pstate page table that will be used to modify the page table of the pstate 
        we want to map */
        struct k_mem_pentry_t *active_ptable_entry = K_MEM_ACTIVE_PSTATE_PTABLE_MAP_POINTER + dir_index;

        active_pdir_entry->entry = pstate->ptable_page | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_PAGE_CACHE_DISABLE;
        active_ptable_entry->entry = active_pdir_entry->entry;

        /* this address allows us to modify the contents of the physical page that backs the page table
        of the pstate we want to map. We'll need that in case this pstate isn't already initialized */
        struct k_mem_pentry_t *active_pdir_entry_ptable = K_MEM_ACTIVE_PSTATE_PDIR_PTABLES_MAP_POINTER[dir_index].entries;
        k_mem_InvalidateTLB((uint32_t)active_pdir_entry_ptable);

        // mapped_pstate = (struct k_mem_pstate_p *)(map_address + (K_MEM_4KB_ADDRESS_OFFSET * K_MEM_PSTATE_SELF_PTABLE_INDEX));

        /* those addresses are kind of like the magic pointers we use to modify the active pstate contents */
        struct k_mem_pentry_t *mapped_pdir = (struct k_mem_pentry_t *)K_MEM_PSTATE_PDIR_MAP_ADDRESS(map_address);
        struct k_mem_pentry_t *mapped_ptable = (struct k_mem_pentry_t *)K_MEM_PSTATE_PTABLE_MAP_ADDRESS(map_address);
        struct k_mem_pentry_page_t *mapped_pdir_ptables = (struct k_mem_pentry_page_t *)K_MEM_PSTATE_PDIR_PTABLES_MAP_ADDRESS(map_address);

        if((active_pdir_entry_ptable[K_MEM_PSTATE_PTABLE_PTABLE_INDEX].entry & K_MEM_PENTRY_ADDR_MASK) != pstate->ptable_page)
        {
            /* this pstate hasn't been initialized, so we set up the necessary stuff here */

            for(uint32_t entry_index = 0; entry_index < K_MEM_PSTATE_SELF_PTABLE_INDEX; entry_index++)
            {
                active_pdir_entry_ptable[entry_index].entry = 0;    
            }

            uint32_t flags = K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_PAGE_CACHE_DISABLE;

            active_pdir_entry_ptable[K_MEM_PSTATE_SELF_PTABLE_INDEX].entry = pstate->self_page | flags;
            active_pdir_entry_ptable[K_MEM_PSTATE_PDIR_PTABLE_INDEX].entry = pstate->pdir_page | flags;
            active_pdir_entry_ptable[K_MEM_PSTATE_PTABLE_PTABLE_INDEX].entry = pstate->ptable_page | flags;

            for(uint32_t entry_index = 0; entry_index <= K_MEM_PSTATE_LAST_USABLE_PDIR_INDEX; entry_index++)
            {
                mapped_pdir[entry_index].entry = 0;    
            }

            mapped_pdir[K_MEM_PSTATE_SELF_PDIR_INDEX].entry = active_pdir_entry->entry;
        }

        // mapped_pstate = (struct k_mem_pstate_p *)(map_address + (K_MEM_4KB_ADDRESS_OFFSET * K_MEM_PSTATE_SELF_PTABLE_INDEX));
        // k_mem_invalidate_tlb((uint32_t)mapped_pstate);
        mapped_pstate->page_dir = mapped_pdir;
        mapped_pstate->page_table = mapped_ptable;
        mapped_pstate->page_dir_ptables = mapped_pdir_ptables;

        k_mem_InvalidateTLB((uint32_t)mapped_pstate->page_dir);
        k_mem_InvalidateTLB((uint32_t)mapped_pstate->page_table);
        k_mem_InvalidateTLB((uint32_t)mapped_pstate->page_dir_ptables);
    }
}

void k_mem_UnmapPState(struct k_mem_pstate_t *mapped_pstate)
{
    uint32_t dir_index = K_MEM_PDIR_INDEX((uint32_t)mapped_pstate->page_dir_ptables);
    uint32_t map_address = K_MEM_4MB_ADDRESS_OFFSET * dir_index;

    if(mapped_pstate->page_table != K_MEM_ACTIVE_PSTATE_PTABLE_MAP_POINTER)
    {
        K_MEM_ACTIVE_PSTATE_PDIR_MAP_POINTER[dir_index].entry = 0;

        k_mem_release_block(map_address);

        for(uint32_t page_index = 0; page_index < 1024; page_index++)
        {
            k_mem_InvalidateTLB(map_address);
            map_address += K_MEM_4KB_ADDRESS_OFFSET;
        }
    }
}

void k_mem_DestroyPState(struct k_mem_pstate_t *pstate)
{
    (void)pstate;
}

void k_mem_LoadPState(struct k_mem_pstate_h *pstate)
{
    k_mem_LoadPageDir(pstate->pdir_page);

    // struct k_mem_pstate_p *mapped_pstate = K_MEM_ACTIVE_PSTATE;

    // mapped_pstate->page_dir = K_MEM_ACTIVE_PSTATE_PDIR;
    // mapped_pstate->page_dir_ptables = K_MEM_ACTIVE_PSTATE_PDIR_PTABLES;
    // mapped_pstate->page_table = K_MEM_ACTIVE_PSTATE_PTABLE;
}

uint32_t k_mem_MapAddress(struct k_mem_pstate_t *mapped_pstate, uint32_t phys_address, uint32_t lin_address, uint32_t flags)
{
    uint32_t dir_index = K_MEM_PDIR_INDEX(lin_address);
    uint32_t table_index = K_MEM_PTABLE_INDEX(lin_address);
    struct k_mem_pentry_t *pentry = NULL;

    if(dir_index == K_MEM_PSTATE_SELF_PDIR_INDEX)
    {
        return K_MEM_PAGING_STATUS_NOT_ALLOWED;
    }

    pentry = mapped_pstate->page_dir + dir_index;

    /* TODO: handle entries marked as not present */

    if(pentry->entry & K_MEM_PENTRY_FLAG_USED)
    {
        if(pentry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE)
        {
            /* this entry already maps an address to a big page */
            return K_MEM_PAGING_STATUS_ALREADY_USED;
        }

        pentry = mapped_pstate->page_dir_ptables[dir_index].entries + table_index;

        if(pentry->entry & K_MEM_PENTRY_FLAG_USED) 
        {
            /* this entry already maps an address to a normal page */
            return K_MEM_PAGING_STATUS_ALREADY_USED;
        }

        /* this the page directory entry references a page table and now we have a pointer to it */
    }
    else
    {
        /* the page directory entry is not being used, so we need to check what size of page we'll be mapping. If it's 
        a big page, all we have to do is point the directory entry to the physical page and set some flags. Otherwise, 
        we'll be mapping a normal page, and we'll need to first allocate a page to hold the directory's page table, map 
        this page, then set up the physical page address and flags we want to actually map. */

        if(!(flags & K_MEM_PENTRY_FLAG_BIG_PAGE) && (pentry->entry & K_MEM_PENTRY_ADDR_MASK) == 0)
        {
            /* we'll be mapping a normal page and the page directory doesn't have a page table, so we 
            allocate and map a physical page that will hold the page table we need. */

            uint32_t page_table = k_mem_AllocPage(0);

            pentry->entry = page_table | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_PAGE_CACHE_DISABLE;
            mapped_pstate->page_table[dir_index].entry = pentry->entry;
            pentry = mapped_pstate->page_dir_ptables[dir_index].entries;

            for(uint32_t entry_index = 0; entry_index < 1024; entry_index++)
            {
                k_mem_InvalidateTLB((uint32_t)(pentry + entry_index));
                pentry[entry_index].entry = 0;
            }

            pentry += table_index;
        }
    }
    
    pentry->entry = phys_address | flags | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED;
    k_mem_InvalidateTLB(lin_address);

    return K_MEM_PAGING_STATUS_OK;
}

uint32_t k_mem_IsAddressMapped(struct k_mem_pstate_t *mapped_pstate, uint32_t address)
{
    uint32_t dir_index = K_MEM_PDIR_INDEX(address);
    struct k_mem_pentry_t *page_entry = NULL;

    page_entry = mapped_pstate->page_dir + dir_index;

    if(page_entry->entry & K_MEM_PENTRY_FLAG_USED)
    {
        if(page_entry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE)
        {
            /* this entry is used and is mapping a big page */
            return 1;
        }

        /* this entry points to a page table, so we'll check that now */
        uint32_t table_index = K_MEM_PTABLE_INDEX(address);
        page_entry = mapped_pstate->page_dir_ptables[dir_index].entries + table_index;

        if(page_entry->entry & K_MEM_PENTRY_FLAG_USED)
        {
            return 1;
        }
    }

    /* if the page directory entry is not used, it's guaranteed that the address is not 
    mapped in any way */
    return 0;
}

uint32_t k_mem_IsPageResident(uint32_t address)
{
    // struct k_mem_pstate_p *pstate = K_MEM_ACTIVE_PSTATE;
    uint32_t pdir_index = K_MEM_PDIR_INDEX(address);
    uint32_t ptable_index = K_MEM_PTABLE_INDEX(address);

    struct k_mem_pentry_t *entry = K_MEM_ACTIVE_PSTATE_PDIR_MAP_POINTER + pdir_index;
    
    if(entry->entry & K_MEM_PENTRY_FLAG_USED)
    {
        if(!(entry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE))
        {
            entry = K_MEM_ACTIVE_PSTATE_PDIR_PTABLES_MAP_POINTER[pdir_index].entries + ptable_index;
        }
    }

    return entry->entry & K_MEM_PENTRY_FLAG_PRESENT;
}

void k_mem_MakePageResident(uint32_t address)
{
    if(!k_mem_IsPageResident(address))
    {
        uint32_t page = k_mem_AllocPage(0);
        k_mem_MapAddress(&K_MEM_ACTIVE_MAPPED_PSTATE, page, address, K_MEM_PENTRY_FLAG_READ_WRITE);
    }
}

uint32_t k_mem_UnmapAddress(struct k_mem_pstate_t *mapped_pstate, uint32_t lin_address)
{    
    uint32_t dir_index = K_MEM_PDIR_INDEX(lin_address);
    uint32_t table_index = K_MEM_PTABLE_INDEX(lin_address);
    struct k_mem_pentry_t *page_entry = NULL;

    if(dir_index == K_MEM_PSTATE_SELF_PDIR_INDEX)
    {
        return K_MEM_PAGING_STATUS_NOT_ALLOWED;
    }

    page_entry = mapped_pstate->page_dir + dir_index;

    if(!(page_entry->entry & K_MEM_PENTRY_FLAG_USED))
    {
        /* not being used for a big page/doesn't reference a page table */
        return K_MEM_PAGING_STATUS_NOT_PAGED;
    }

    if(!(page_entry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE))
    {
        /* this page directory references a page table */

        page_entry = mapped_pstate->page_dir_ptables[dir_index].entries + table_index;

        if(!(page_entry->entry & K_MEM_PENTRY_FLAG_USED))
        {
            /* address not mapped */
            return K_MEM_PAGING_STATUS_NOT_PAGED;
        }
    }

    page_entry->entry = 0;

    k_mem_InvalidateTLB(lin_address);

    return K_MEM_PAGING_STATUS_OK;
}

// struct k_mem_pentry_t k_mem_address_pentry(struct k_mem_pstate_t *mapped_pstate, uint32_t lin_address)
// {
//     uint32_t pdir_index = K_MEM_PDIR_INDEX(lin_address);
//     uint32_t ptable_index = K_MEM_PTABLE_INDEX(lin_address);
//     struct k_mem_pentry_t entry = {};

//     struct k_mem_pentry_t *active_entry = mapped_pstate->page_dir + pdir_index;

//     if(active_entry->entry & K_MEM_PENTRY_FLAG_USED)
//     {
//         if(!(active_entry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE))
//         {
//             active_entry = mapped_pstate->page_dir_ptables[pdir_index].entries + ptable_index;
//         }

//         entry.entry = active_entry->entry;
//     }

//     return entry;
// }

void k_mem_add_block(uint32_t block_address, uint32_t block_size)
{
    (void)block_address;
    (void)block_size;
    // if(block_size >= sizeof(struct k_mem_block_t) + K_MEM_MIN_ALLOC_SIZE)
    // {
    //     block_size -= sizeof(struct k_mem_block_t);

    //     k_mem_make_page_resident(block_address);

    //     // if(!k_mem_is_address_mapped(K_MEM_ACTIVE_PSTATE, block_address))
    //     // {
    //     //     uint32_t page = k_mem_alloc_page();
    //     //     k_mem_map_address(K_MEM_ACTIVE_PSTATE, page, block_address, K_MEM_PENTRY_FLAG_READ_WRITE);
    //     // }

    //     uint32_t next_page_address = (block_address & K_MEM_PENTRY_ADDR_MASK) + K_MEM_4KB_ADDRESS_OFFSET;

    //     if(next_page_address - block_address < sizeof(struct k_mem_block_t))
    //     {
    //         k_mem_make_page_resident(next_page_address);
    //     }

    //     struct k_mem_block_t *block = (struct k_mem_block_t *)block_address;
    //     block->size = block_size;
    //     block->next = NULL;
    //     block->prev = NULL;

    //     k_mem_release_block((uint32_t)(block + 1));
    // }
}

uint32_t k_mem_reserve_block(uint32_t size, uint32_t align)
{
    (void)size;
    (void)align;
    // struct k_mem_pstate_p *pstate = K_MEM_ACTIVE_PSTATE;
    // struct k_mem_heapm_t *heap = &pstate->heapm;
    // struct k_mem_block_t *block = heap->blocks; 

    // if(!align)
    // {
    //     align = 1;
    // }

    // if(size < K_MEM_MIN_ALLOC_SIZE)
    // {
    //     size = K_MEM_MIN_ALLOC_SIZE;
    // }

    // while(block)
    // {
    //     uint32_t block_address = (uint32_t)(block + 1);
    //     uint32_t address_align = block_address % align;
    //     uint32_t block_size = block->size;

    //     if(address_align)
    //     {
    //         address_align = align - address_align;
    //         block_address += address_align;
    //         block_size -= address_align;
    //     }
        
    //     if(block_size >= size)
    //     {
    //         if(block_size > size)
    //         {
    //             uint32_t new_block_address = block_address + size;
    //             address_align = new_block_address % sizeof(struct k_mem_block_t );

    //             if(address_align)
    //             {
    //                 address_align = sizeof(struct k_mem_block_t ) - address_align;
    //                 new_block_address += address_align;
    //                 size += address_align;
    //             }

    //             uint32_t new_block_size = block_size - size;

    //             if(size < block_size && new_block_size >= sizeof(struct k_mem_block_t) + K_MEM_MIN_ALLOC_SIZE)
    //             {
    //                 k_mem_make_page_resident(new_block_address);

    //                 struct k_mem_block_t *new_block = (struct k_mem_block_t *)new_block_address;
    //                 new_block->size = new_block_size - sizeof(struct k_mem_block_t);
    //                 new_block->next = block->next;
    //                 if(new_block->next) 
    //                 {
    //                     new_block->next->prev = new_block;
    //                 }

    //                 new_block->prev = block;
    //                 block->next = new_block;
    //             }
    //         }

    //         block->size = size;

    //         if(block->prev)
    //         {
    //             block->prev->next = block->next;
    //         }
    //         else
    //         {
    //             heap->blocks = block->next;
    //         }

    //         if(block->next)
    //         {
    //             block->next->prev = block->prev;
    //         }
    //         else
    //         {
    //             heap->last_block = block->prev;
    //         }

    //         block->next = NULL;
    //         block->prev = NULL;

    //         return block_address;
    //     }

    //     block = block->next;
    // }

    return 0;
}

void k_mem_release_block(uint32_t address)
{
    (void)address;
    // struct k_mem_pstate_p *pstate = K_MEM_ACTIVE_PSTATE;
    // struct k_mem_heapm_t *heap = &pstate->heapm;
    // struct k_mem_block_t *block = (struct k_mem_block_t *)address - 1;

    // block->next = NULL;
    // block->prev = NULL;

    // if(!heap->blocks)
    // {
    //     heap->blocks = block;
    // }
    // else
    // {
    //     block->prev = heap->last_block;
    //     heap->last_block->next = block;
    // }

    // heap->last_block = block;
    // heap->block_count++;
}

void *k_mem_alloc(uint32_t size, uint32_t align)
{
    // uint32_t block_address = k_mem_reserve_block(size, align);
    // k_mem_make_alloc_resident((void *)block_address);
    // return (void *)block_address;
    (void)size;
    (void)align;

    return NULL;
}

void k_mem_make_alloc_resident(void *memory)
{
    (void)memory;
    // if(memory)
    // {
    //     uint32_t alloc_address = (uint32_t)memory;
    //     struct k_mem_block_t *block = (struct k_mem_block_t *)memory - 1;
    //     k_mem_make_page_resident((uint32_t)block);

    //     uint32_t first_page = alloc_address & K_MEM_SMALL_PAGE_ADDR_MASK;
    //     uint32_t last_page = (alloc_address + block->size) & K_MEM_SMALL_PAGE_ADDR_MASK;

    //     while(first_page != last_page)
    //     {
    //         k_mem_make_page_resident(first_page);
    //         first_page += K_MEM_4KB_ADDRESS_OFFSET;
    //     }
    // }
}

void k_mem_free(void *memory) 
{
    (void)memory;
    // if(memory)
    // {
    //     struct k_mem_block_t *block = (struct k_mem_block_t *)memory - 1;


    // }
}