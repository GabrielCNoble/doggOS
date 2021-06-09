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

extern struct k_mem_pentry_t k_mem_pdirs[];
// struct k_mem_pentry_t *k_mem_ptables;
struct k_mem_pstate_t k_mem_pstate;

extern uint32_t k_kernel_start;
extern uint32_t k_kernel_end;

uint8_t *k_mem_used_pages;
struct k_mem_pentry_t *k_mem_avail_pages;
uint32_t k_mem_avail_pages_cursor;
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



    uint32_t bits_per_page = 4096 * 8;
    covered_bytes_block_size = 4096 * (bits_per_page / K_MEM_USED_PAGE_BITS) + 4096;
    /* to simplify mapping page addresses to its proper byte/bit, we'll allocate
    enough bytes to cover the whole 4GB address space */
    uint32_t used_page_pages = 0xffffffff / covered_bytes_block_size;
    if(0xffffffff % covered_bytes_block_size)
    {
        used_page_pages++;
    }
    uint32_t used_page_bytes = used_page_pages * 4096;
    
    /* The page directory has 1024 entries in 32 bit paging mode, and each 
    page table has 1024 page entries. */
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


    k_mem_avail_pages = (struct k_mem_pentry_t *)((uint32_t)lowest_range->base);
    lowest_range->base += free_page_header_bytes;
    k_mem_used_pages = (uint8_t *)((uint32_t)lowest_range->base);
    lowest_range->base += used_page_bytes;

    uint32_t data_end = lowest_range->base;
    uint32_t align = data_end % 4096;

    if(align)
    {
        align = 4096 - align;
        data_end += align;
    }

    if(lowest_range->size > total_size)
    {
        /* the range we used to store our data still has some
        space left */
        lowest_range->size -= total_size;
    }
    else
    {
        /* the range was completely used, so drop it from the list */
        k_mem_ranges[lowest_range_index] = k_mem_ranges[k_mem_range_count];
        k_mem_range_count--;
    }

    k_mem_avail_pages_cursor = 0xffffffff;
    
    for(uint32_t range_index = 0; range_index < k_mem_range_count; range_index++)
    {
        struct k_mem_range_t *range = k_mem_ranges + range_index;
        uint32_t page_address = (uint32_t)range->base;
        uint32_t page_count = (uint32_t)range->size / 4096;

        for(uint32_t page_index = 0; page_index < page_count; page_index++)
        {
            k_mem_avail_pages_cursor++;
            struct k_mem_pentry_t *entry = k_mem_avail_pages + k_mem_avail_pages_cursor;
            entry->entry = page_address;
            page_address += 4096;
        }
    }

    /* to simplify things further down the road we set up a really simple page state here. 
    All but the last page dir entry will map 4MB pages. The last entry will contain a page
    table, which will map the pages containing this page dir and this page table. This 'kinda'
    page state will be thrown away afterwards. */
    struct k_mem_pentry_t *page_dir = (struct k_mem_pentry_t *)k_mem_alloc_page();
    struct k_mem_pentry_t *page_table = (struct k_mem_pentry_t *)k_mem_alloc_page();
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
        address += 0x00400000;
    }

    k_mem_load_page_dir((uint32_t)page_dir);
    k_mem_enable_paging();


    /* we need to create the kernel pstate sort of manually here, because the "full" mechanism relies on the 
    heap manager to get an exclusive 4MB block, but right now there isn't a heap manager in any address space,
    so we use a fixed address that's guaranteed to not be in use right now. This mechanism could be made to 
    always use this fixed address, but that either would require a lot of synchronization, because more than one
    core might be trying to map a pstate while running kernel code, or several fixed 4MB blocks will need to be
    set aside (one for each core) to get rid of syncronization, which is not terrible, but not great either.  */

    uint32_t map_address = K_MEM_4MB_ADDRESS_OFFSET * K_MEM_PSTATE_LAST_USABLE_PDIR_INDEX;
    k_mem_pstate.self_page = k_mem_alloc_page();
    k_mem_pstate.pdir_page = k_mem_alloc_page();
    k_mem_pstate.ptable_page = k_mem_alloc_page();

    /* this will both map and initialize the page directory and page table for us */
    struct k_mem_pstate_p *mapped_pstate = k_mem_map_pstate_to_address(&k_mem_pstate, map_address);
    address = 0;

    while(address < data_end)
    {
        k_mem_map_address(mapped_pstate, address, address, K_MEM_PENTRY_FLAG_READ_WRITE);
        address += K_MEM_4KB_ADDRESS_OFFSET;
    }

    k_mem_load_pstate(&k_mem_pstate);
    for(uint32_t range_index = 0; range_index < k_mem_range_count; range_index++)
    {
        struct k_mem_range_t *range = k_mem_ranges + range_index;
        k_mem_add_block(range->base, range->size);
    }

    /* from now on, all the pstate creation and manipulation is available. Also, we purposefully won't unmap
    the kernel pstate here, because we used a fixed address */

    k_mem_free_pages((uint32_t)page_dir);
    k_mem_free_pages((uint32_t)page_table);
}

uint32_t k_mem_alloc_page()
{
    uint32_t page_address = K_MEM_INVALID_PAGE;

    if(k_mem_avail_pages_cursor != 0xffffffff)
    {
        struct k_mem_pentry_t *entry = k_mem_avail_pages + k_mem_avail_pages_cursor;
        k_mem_avail_pages_cursor--;
        uint32_t byte_index = K_MEM_SMALL_PAGE_USED_BYTE_INDEX(entry->entry);
        uint32_t bit_index = K_MEM_SMALL_PAGE_USED_BIT_INDEX(entry->entry);
        k_mem_used_pages[byte_index] |= (K_MEM_PAGE_FLAG_USED | K_MEM_PAGE_FLAG_HEAD) << bit_index;
        page_address = (entry->entry & K_MEM_SMALL_PAGE_ADDR_MASK);
    }

    return page_address;
}

void k_mem_sort_entries(uint32_t start, uint32_t end, struct k_mem_pentry_t *entries)
{
    uint32_t left = start;
    uint32_t right = end;
    uint32_t middle = (right + left) / 2;
    struct k_mem_pentry_t *middle_entry = entries + middle;
    uint32_t middle_entry_page = middle_entry->entry & K_MEM_SMALL_PAGE_ADDR_MASK;

    do
    {
        while((entries[left].entry & K_MEM_SMALL_PAGE_ADDR_MASK) > middle_entry_page && left < end) left++;
        while((entries[right].entry & K_MEM_SMALL_PAGE_ADDR_MASK) < middle_entry_page && right > start) right--;

        if(left <= right)
        {
            struct k_mem_pentry_t temp_entry = entries[left];
            entries[left] = entries[right];
            entries[right] = temp_entry;
            left++;
            right--;
        }
    }
    while(left <= right);

    if(start < right)
    {
        k_mem_sort_entries(start, right, entries);
    }
    
    if(left < end)
    {
        k_mem_sort_entries(left, end, entries);
    }
}

uint32_t k_mem_alloc_pages(uint32_t count)
{
    uint32_t page_address = K_MEM_INVALID_PAGE;
    uint32_t entries_sorted = 0;
    if(k_mem_avail_pages_cursor != 0xffffffff && k_mem_avail_pages_cursor >= count)
    {
        _try_again:

        uint32_t entry_count = k_mem_avail_pages_cursor - count;
        for(uint32_t entry_index = 0; entry_index <= entry_count; entry_index++)
        {
            struct k_mem_pentry_t *head_entry = k_mem_avail_pages + entry_index;
            struct k_mem_pentry_t *first_entry = head_entry;

            for(uint32_t next_entry_index = 1; next_entry_index < count; next_entry_index++)
            {
                struct k_mem_pentry_t *second_entry = head_entry + next_entry_index;
                uint32_t second_entry_page = second_entry->entry & K_MEM_SMALL_PAGE_ADDR_MASK;
                uint32_t first_entry_page = first_entry->entry & K_MEM_SMALL_PAGE_ADDR_MASK;
                /* likely not the ideal thing to do here. Could be more efficient to try
                both combinations, and swap the entries if those point to adjacent pages */
                if(first_entry_page > second_entry_page || second_entry_page - first_entry_page > 4096)
                {
                    head_entry = NULL;
                    break; 
                }

                first_entry = second_entry;
            }

            if(head_entry)
            {
                uint32_t byte_index = K_MEM_SMALL_PAGE_USED_BYTE_INDEX(head_entry->entry);
                uint32_t bit_index = K_MEM_SMALL_PAGE_USED_BIT_INDEX(head_entry->entry);
                uint32_t flags = (K_MEM_PAGE_FLAG_USED | K_MEM_PAGE_FLAG_HEAD) << bit_index;
                k_mem_used_pages[byte_index] |= flags;

                for(uint32_t entry_index = 1; entry_index < count; entry_index++)
                {
                    struct k_mem_pentry_t *next_entry = head_entry + entry_index;
                    byte_index = K_MEM_SMALL_PAGE_USED_BYTE_INDEX(next_entry->entry);
                    bit_index = K_MEM_SMALL_PAGE_USED_BIT_INDEX(next_entry->entry);
                    flags = K_MEM_PAGE_FLAG_USED << bit_index;
                    k_mem_used_pages[byte_index] |= flags;
                }

                page_address = head_entry->entry & K_MEM_SMALL_PAGE_ADDR_MASK;
                break;
            }
        }

        if(page_address == K_MEM_INVALID_PAGE && !entries_sorted)
        {
            k_mem_sort_entries(0, k_mem_avail_pages_cursor + 1, k_mem_avail_pages);
            entries_sorted = 1;
            goto _try_again;
        }
    }

    return page_address;
}

void k_mem_free_pages(uint32_t page_address)
{
    if(page_address != K_MEM_INVALID_PAGE)
    {
        uint32_t byte_index = K_MEM_SMALL_PAGE_USED_BYTE_INDEX(page_address);
        uint32_t bit_index = K_MEM_SMALL_PAGE_USED_BIT_INDEX(page_address);
        uint32_t valid_bits = (K_MEM_PAGE_FLAG_USED | K_MEM_PAGE_FLAG_HEAD) << bit_index;

        if((k_mem_used_pages[byte_index] & valid_bits) == valid_bits)
        {
            do
            {
                k_mem_used_pages[byte_index] &= ~valid_bits;
                k_mem_avail_pages_cursor++;
                struct k_mem_pentry_t *entry = k_mem_avail_pages + k_mem_avail_pages_cursor;
                entry->entry = page_address; 
                page_address += 4096;
                byte_index = K_MEM_SMALL_PAGE_USED_BYTE_INDEX(page_address);
                bit_index = K_MEM_SMALL_PAGE_USED_BIT_INDEX(page_address);
                valid_bits = (K_MEM_PAGE_FLAG_USED | K_MEM_PAGE_FLAG_HEAD) << bit_index;
            }
            while((k_mem_used_pages[byte_index] & valid_bits) == (K_MEM_PAGE_FLAG_USED << bit_index));
        }
    }
}

/*===========================================================================*/
/*===========================================================================*/
/*===========================================================================*/
struct k_mem_pstate_t k_mem_create_pstate()
{
    struct k_mem_pstate_t pstate = {};

    pstate.self_page = k_mem_alloc_page();
    pstate.pdir_page = k_mem_alloc_page();
    pstate.ptable_page = k_mem_alloc_page();

    /* this will initialize this pstate page directory and page table accordingly */
    struct k_mem_pstate_p *mapped_pstate = k_mem_map_pstate(&pstate);
    /* the only "bad" side effect is that we need to unmap it right afterwards, to not
    leak a 4MB block */
    k_mem_unmap_pstate(mapped_pstate);

    return pstate;
}

struct k_mem_pstate_p *k_mem_map_pstate(struct k_mem_pstate_t *pstate)
{
    /* 4MB aligned block, so we can be sure it'll be contained in a single page
    directory. We're only reserving a portion of the linear memory space here, no
    physical memory is being commited */
    uint32_t map_address = k_mem_reserve_block(0x00400000, 0x00400000);

    return k_mem_map_pstate_to_address(pstate, map_address);
}

struct k_mem_pstate_p *k_mem_map_pstate_to_address(struct k_mem_pstate_t *pstate, uint32_t map_address)
{
    uint32_t dir_index = K_MEM_PDIR_INDEX(map_address);
    struct k_mem_pstate_p *mapped_pstate = NULL;

    if(!(map_address & 0x003fffff))
    {
        /* only map this pstate if the address is aligned to a 4MB boundary */


        /* page dir entry in the active pstate that will use the page table of the pstate we want to map */
        struct k_mem_pentry_t *active_pdir_entry = K_MEM_ACTIVE_PSTATE_PDIR + dir_index;

        /* entry in the active pstate page table that will be used to modify the page table of the pstate 
        we want to map */
        struct k_mem_pentry_t *active_ptable_entry = K_MEM_ACTIVE_PSTATE_PTABLE + dir_index;

        active_pdir_entry->entry = pstate->ptable_page | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE;
        active_ptable_entry->entry = active_pdir_entry->entry;

        struct k_mem_pentry_t *pstate_ptable = K_MEM_ACTIVE_PSTATE_PDIR_PTABLES[dir_index].entries;
        k_mem_invalidate_tlb((uint32_t)pstate_ptable);

        struct k_mem_pentry_t *mapped_pdir = (struct k_mem_pentry_t *)(map_address + (K_MEM_4KB_ADDRESS_OFFSET * K_MEM_PSTATE_PDIR_PTABLE_INDEX));
        struct k_mem_pentry_t *mapped_ptable = (struct k_mem_pentry_t *)(map_address + (K_MEM_4KB_ADDRESS_OFFSET * K_MEM_PSTATE_PTABLE_PTABLE_INDEX));
        struct k_mem_pentry_page_t *mapped_pdir_ptables = (struct k_mem_pentry_page_t *)map_address;

        if((pstate_ptable[K_MEM_PSTATE_PTABLE_PTABLE_INDEX].entry & K_MEM_PENTRY_ADDR_MASK) != pstate->ptable_page)
        {
            for(uint32_t entry_index = 0; entry_index < K_MEM_PSTATE_SELF_PTABLE_INDEX; entry_index++)
            {
                pstate_ptable[entry_index].entry = 0;    
            }

            /* this pstate hasn't been initialized, so we set up the necessary stuff here */
            pstate_ptable[K_MEM_PSTATE_SELF_PTABLE_INDEX].entry = pstate->self_page | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE;
            pstate_ptable[K_MEM_PSTATE_PDIR_PTABLE_INDEX].entry = pstate->pdir_page | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE;
            pstate_ptable[K_MEM_PSTATE_PTABLE_PTABLE_INDEX].entry = pstate->ptable_page | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE;

            for(uint32_t entry_index = 0; entry_index <= K_MEM_PSTATE_LAST_USABLE_PDIR_INDEX; entry_index++)
            {
                mapped_pdir[entry_index].entry = 0;    
            }

            mapped_pdir[K_MEM_PSTATE_SELF_PDIR_INDEX].entry = active_pdir_entry->entry;
        }

        mapped_pstate = (struct k_mem_pstate_p *)(map_address + (K_MEM_4KB_ADDRESS_OFFSET * K_MEM_PSTATE_SELF_PTABLE_INDEX));
        k_mem_invalidate_tlb((uint32_t)mapped_pstate);
        mapped_pstate->page_dir = mapped_pdir;
        mapped_pstate->page_table = mapped_ptable;
        mapped_pstate->page_dir_ptables = mapped_pdir_ptables;

        k_mem_invalidate_tlb((uint32_t)mapped_pstate->page_dir);
        k_mem_invalidate_tlb((uint32_t)mapped_pstate->page_table);
        k_mem_invalidate_tlb((uint32_t)mapped_pstate->page_dir_ptables);
    }

    return mapped_pstate;
}

void k_mem_unmap_pstate(struct k_mem_pstate_p *pstate)
{
    uint32_t dir_index = K_MEM_PDIR_INDEX((uint32_t)pstate);
    uint32_t map_address = K_MEM_4MB_ADDRESS_OFFSET * dir_index;

    if(pstate != K_MEM_ACTIVE_PSTATE)
    {
        struct k_mem_pstate_p *active_pstate = K_MEM_ACTIVE_PSTATE;
        struct k_mem_pentry_t *pentry = active_pstate->page_dir + dir_index;
        pentry->entry = 0;

        k_mem_release_block(map_address);

        for(uint32_t page_index = 0; page_index < 1024; page_index++)
        {
            k_mem_invalidate_tlb(map_address);
            map_address += K_MEM_4KB_ADDRESS_OFFSET;
        }
    }
}

void k_mem_destroy_pstate(struct k_mem_pstate_t *pstate)
{

}

void k_mem_load_pstate(struct k_mem_pstate_t *pstate)
{
    k_mem_load_page_dir(pstate->pdir_page);

    struct k_mem_pstate_p *mapped_pstate = K_MEM_ACTIVE_PSTATE;

    mapped_pstate->page_dir = K_MEM_ACTIVE_PSTATE_PDIR;
    mapped_pstate->page_dir_ptables = K_MEM_ACTIVE_PSTATE_PDIR_PTABLES;
    mapped_pstate->page_table = K_MEM_ACTIVE_PSTATE_PTABLE;
}

uint32_t k_mem_map_address(struct k_mem_pstate_p *pstate, uint32_t phys_address, uint32_t lin_address, uint32_t flags)
{
    uint32_t dir_index = K_MEM_PDIR_INDEX(lin_address);
    uint32_t table_index = K_MEM_PTABLE_INDEX(lin_address);
    struct k_mem_pentry_t *pentry = NULL;

    if(dir_index == K_MEM_PSTATE_SELF_PDIR_INDEX)
    {
        return K_MEM_PAGING_STATUS_NOT_ALLOWED;
    }

    pentry = pstate->page_dir + dir_index;

    /* TODO: handle entries marked as not present */

    if(pentry->entry & K_MEM_PENTRY_FLAG_USED)
    {
        if(pentry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE)
        {
            /* this entry already maps an address to a big page */
            return K_MEM_PAGING_STATUS_ALREADY_USED;
        }

        pentry = pstate->page_dir_ptables[dir_index].entries + table_index;

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

            uint32_t page_table = k_mem_alloc_page();

            pentry->entry = page_table | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_READ_WRITE;
            pstate->page_table[dir_index].entry = pentry->entry;
            pentry = pstate->page_dir_ptables[dir_index].entries;

            for(uint32_t entry_index = 0; entry_index < 1024; entry_index++)
            {
                k_mem_invalidate_tlb((uint32_t)(pentry + entry_index));
                pentry[entry_index].entry = 0;
            }

            pentry += table_index;
        }
    }
    
    pentry->entry = phys_address | flags | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED;
    k_mem_invalidate_tlb(lin_address);

    return K_MEM_PAGING_STATUS_OK;
}

uint32_t k_mem_address_mapped(struct k_mem_pstate_p *pstate, uint32_t address)
{
    uint32_t dir_index = K_MEM_PDIR_INDEX(address);
    struct k_mem_pentry_t *page_entry = NULL;

    page_entry = pstate->page_dir + dir_index;

    if(page_entry->entry & K_MEM_PENTRY_FLAG_USED)
    {
        if(page_entry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE)
        {
            /* this entry is used and is mapping a big page */
            return 1;
        }

        /* this entry points to a page table, so we'll check that now */
        uint32_t table_index = K_MEM_PTABLE_INDEX(address);
        page_entry = pstate->page_dir_ptables[dir_index].entries + table_index;

        if(page_entry->entry & K_MEM_PENTRY_FLAG_USED)
        {
            return 1;
        }
    }

    /* if the page directory entry is not used, it's guaranteed that the address is not 
    mapped in any way */
    return 0;
}

uint32_t k_mem_unmap_address(struct k_mem_pstate_p *pstate, uint32_t lin_address)
{    
    uint32_t dir_index = K_MEM_PDIR_INDEX(lin_address);
    uint32_t table_index = K_MEM_PTABLE_INDEX(lin_address);
    struct k_mem_pentry_t *page_entry = NULL;

    if(dir_index == K_MEM_PSTATE_SELF_PDIR_INDEX)
    {
        return K_MEM_PAGING_STATUS_NOT_ALLOWED;
    }

    page_entry = pstate->page_dir + dir_index;

    if(!(page_entry->entry & K_MEM_PENTRY_FLAG_USED))
    {
        /* not being used for a big page/doesn't reference a page table */
        return K_MEM_PAGING_STATUS_NOT_PAGED;
    }

    if(!(page_entry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE))
    {
        /* this page directory references a page table */

        page_entry = pstate->page_dir_ptables[dir_index].entries + table_index;

        if(!(page_entry->entry & K_MEM_PENTRY_FLAG_USED))
        {
            /* address not mapped */
            return K_MEM_PAGING_STATUS_NOT_PAGED;
        }
    }

    page_entry->entry = 0;

    k_mem_invalidate_tlb(lin_address);

    return K_MEM_PAGING_STATUS_OK;
}

void k_mem_add_block(uint32_t block_address, uint32_t block_size)
{
    
    if(block_size >= sizeof(struct k_mem_block_t) + K_MEM_MIN_ALLOC_SIZE)
    {
        block_size -= sizeof(struct k_mem_block_t);

        if(!k_mem_address_mapped(K_MEM_ACTIVE_PSTATE, block_address))
        {
            uint32_t page = k_mem_alloc_page();
            k_mem_map_address(K_MEM_ACTIVE_PSTATE, page, block_address, K_MEM_PENTRY_FLAG_READ_WRITE);
        }

        uint32_t next_page_address = (block_address & K_MEM_PENTRY_ADDR_MASK) + K_MEM_4KB_ADDRESS_OFFSET;

        if(next_page_address - block_address < sizeof(struct k_mem_block_t))
        {
            uint32_t page = k_mem_alloc_page();
            k_mem_map_address(K_MEM_ACTIVE_PSTATE, page, next_page_address, K_MEM_PENTRY_FLAG_READ_WRITE);
        }

        struct k_mem_block_t *block = (struct k_mem_block_t *)block_address;
        block->size = block_size;
        block->next = NULL;
        block->prev = NULL;

        k_mem_release_block((uint32_t)(block + 1));
    }
}

uint32_t k_mem_reserve_block(uint32_t size, uint32_t align)
{
    struct k_mem_pstate_p *pstate = K_MEM_ACTIVE_PSTATE;
    struct k_mem_heapm_t *heap = &pstate->heapm;
    struct k_mem_block_t *block = heap->blocks; 

    if(!align)
    {
        align = 1;
    }

    if(size < K_MEM_MIN_ALLOC_SIZE)
    {
        size = K_MEM_MIN_ALLOC_SIZE;
    }

    while(block)
    {
        uint32_t block_address = (uint32_t)(block + 1);
        uint32_t address_align = block_address % align;
        uint32_t block_size = block->size;

        if(address_align)
        {
            address_align = align - address_align;
            block_address += address_align;
            block_size -= address_align;
        }
        
        if(block_size >= size)
        {
            if(block_size > size)
            {
                uint32_t new_block_address = block_address + size;
                address_align = new_block_address % sizeof(struct k_mem_block_t );

                if(address_align)
                {
                    address_align = sizeof(struct k_mem_block_t ) - address_align;
                    new_block_address += address_align;
                    size += address_align;
                }

                uint32_t new_block_size = block_size - size;

                if(size < block_size && new_block_size >= sizeof(struct k_mem_block_t) + K_MEM_MIN_ALLOC_SIZE)
                {
                    if(!k_mem_address_mapped(K_MEM_ACTIVE_PSTATE, new_block_address))
                    {
                        uint32_t page = k_mem_alloc_page();
                        k_mem_map_address(K_MEM_ACTIVE_PSTATE, page, new_block_address, K_MEM_PENTRY_FLAG_READ_WRITE);
                    }

                    struct k_mem_block_t *new_block = (struct k_mem_block_t *)new_block_address;
                    new_block->size = new_block_size - sizeof(struct k_mem_block_t);
                    new_block->next = block->next;
                    if(new_block->next) 
                    {
                        new_block->next->prev = new_block;
                    }

                    new_block->prev = block;
                    block->next = new_block;
                }
            }

            block->size = size;

            if(block->prev)
            {
                block->prev->next = block->next;
            }
            else
            {
                heap->blocks = block->next;
            }

            if(block->next)
            {
                block->next->prev = block->prev;
            }
            else
            {
                heap->last_block = block->prev;
            }

            block->next = NULL;
            block->prev = NULL;

            return block_address;
        }

        block = block->next;
    }

    return 0;
}

void k_mem_release_block(uint32_t address)
{
    struct k_mem_pstate_p *pstate = K_MEM_ACTIVE_PSTATE;
    struct k_mem_heapm_t *heap = &pstate->heapm;
    struct k_mem_block_t *block = (struct k_mem_block_t *)address - 1;

    block->next = NULL;
    block->prev = NULL;

    if(!heap->blocks)
    {
        heap->blocks = block;
    }
    else
    {
        block->prev = heap->last_block;
        heap->last_block->next = block;
    }

    heap->last_block = block;
    heap->block_count++;
}

void *k_mem_alloc(uint32_t size, uint32_t align)
{
    uint32_t block_address = k_mem_reserve_block(size, align);
    k_mem_commit(block_address, size);
    return (void *)block_address;
}

uint32_t k_mem_commit(uint32_t address, uint32_t size)
{
    uint32_t first_block_page = (address & K_MEM_PENTRY_ADDR_MASK);
    uint32_t last_block_page = (address + size) & K_MEM_PENTRY_ADDR_MASK;

    if(address)
    {
        for(uint32_t block_page = first_block_page; block_page <= last_block_page; block_page += K_MEM_4KB_ADDRESS_OFFSET)
        {
            if(!k_mem_address_mapped(K_MEM_ACTIVE_PSTATE, block_page))
            {
                uint32_t page = k_mem_alloc_page();
                k_mem_map_address(K_MEM_ACTIVE_PSTATE, page, block_page, K_MEM_PENTRY_FLAG_READ_WRITE);
            }
        }
    }
}

uint32_t k_mem_commit_ctg(uint32_t address, uint32_t size)
{

}

void k_mem_free(void *memory) 
{

    // struct k_mem_heap_t *heap = K_MEM_ACTIVE_PSTATE_HEAP_MANAGER;
    // struct k_mem_block_t *block = (struct k_mem_block_t *)memory - 1;

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