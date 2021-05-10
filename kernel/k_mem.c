#include "k_mem.h"
#include "k_term.h"
#include <stddef.h>


extern struct k_mem_range_t k_mem_low_range;
extern uint32_t k_mem_range_count;
uint32_t k_mem_total = 0;
uint32_t k_mem_reserved = 0;
extern struct k_mem_range_t k_mem_ranges[];

extern uint32_t k_mem_gdt_desc_count;
extern struct k_mem_seg_desc_t k_mem_gdt[]; 

extern struct k_mem_pentry_t k_mem_pdirs[];
// struct k_mem_pentry_t *k_mem_ptables;
struct k_mem_pstate_t *k_mem_pstate;

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

    // uint32_t pages[3];
    // for(uint32_t page_index = 0; page_index < 3; page_index++) 
    // {
    //     pages[page_index] = k_mem_alloc_page();
    // }


    /* to simplify things further down the road we set up a really simple page state here. 
    All but the last page dir entry will map 4MB pages. The last entry will contain a page
    table, which will map the pages containing this page dir and this page table. This 'kinda'
    page state will be thrown away afterwards. */
    struct k_mem_pentry_t *page_dir = (struct k_mem_pentry_t *)k_mem_alloc_page();
    struct k_mem_pentry_t *page_table = (struct k_mem_pentry_t *)k_mem_alloc_page();
    struct k_mem_pentry_t *entry;
    /* map the page that contains the page directory */
    entry = page_table + K_MEM_PSTATE_DIR_PAGE_INDEX;
    entry->entry = ((uint32_t)page_dir) | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_READ_WRITE;
    /* map the page that contains the page table */
    entry = page_table + K_MEM_PSTATE_TABLE_PAGE_INDEX;
    entry->entry = ((uint32_t)page_table) | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_READ_WRITE; 
    /* point the last page directory entry to the page table we just set up */
    entry = page_dir + K_MEM_PSTATE_DIR_INDEX;
    entry->entry = ((uint32_t)page_table) | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_READ_WRITE;
    uint32_t address = 0;

    for(uint32_t entry_index = 0; entry_index < 1023; entry_index++)
    {
        /* identity map the rest of the address space */
        struct k_mem_pentry_t *entry = page_dir + entry_index;
        entry->entry = address | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_BIG_PAGE;
        address += 0x00400000;
    }

    k_mem_load_page_dir(page_dir);
    k_mem_enable_paging();
    /* now all the code that creates/delete/manipulates page states will function properly */

    k_mem_pstate = k_mem_create_pstate();
    address = 0;
    while(address < data_end)
    {
        k_mem_map_address(k_mem_pstate, address, address, K_MEM_PENTRY_FLAG_READ_WRITE);
        address += 0x00001000;
    }
    
    k_mem_load_pstate(k_mem_pstate);
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
struct k_mem_pstate_t *k_mem_create_pstate()
{
    uint32_t pages[3];

    for(uint32_t page_index = 0; page_index < 3; page_index++)
    {
        pages[page_index] = k_mem_alloc_page();
    }

    struct k_mem_pstate_t *pstate = (struct k_mem_pstate_t *)k_mem_map_temp(pages[0]);
    pstate->page_dir = (struct k_mem_pentry_t *)pages[1];
    pstate->last_table = (struct k_mem_pentry_t *)pages[2];

    struct k_mem_pentry_t *self_dir_entry = (struct k_mem_pentry_t *)k_mem_map_temp(pages[1]) + K_MEM_PSTATE_DIR_INDEX;
    self_dir_entry->entry = pages[2] | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE;

    struct k_mem_pentry_t *self_table = ((struct k_mem_pentry_t *)k_mem_map_temp(pages[2])) + K_MEM_PSTATE_LAST_TABLE_FIRST_INDEX;
    for(uint32_t entry_index = 0; entry_index < 3; entry_index++)
    {
        struct k_mem_pentry_t *self_table_entry = self_table + entry_index;
        self_table_entry->entry = pages[entry_index] | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE;
    }

    return (struct k_mem_pstate_t *)pages[0];
}

void k_mem_destroy_pstate(struct k_mem_pstate_t *pstate)
{

}

void k_mem_load_pstate(struct k_mem_pstate_t *pstate)
{
    pstate = (struct k_mem_pstate_t *)k_mem_map_temp((uint32_t)pstate);
    k_mem_load_page_dir(pstate->page_dir);
}

uint32_t k_mem_map_temp(uint32_t phys_address)
{
    struct k_mem_pentry_t *scratch_entry = K_MEM_ACTIVE_PSTATE_TABLE_ADDRESS + K_MEM_PSTATE_TEMP_PAGE_INDEX;
    phys_address = phys_address & K_MEM_SMALL_PAGE_ADDR_MASK;
    scratch_entry->entry = phys_address | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_READ_WRITE;
    k_mem_invalidate_tlb((uint32_t)K_MEM_ACTIVE_PSTATE_TEMP_ADDRESS);
    return (uint32_t)K_MEM_ACTIVE_PSTATE_TEMP_ADDRESS;
}

uint32_t k_mem_map_address(struct k_mem_pstate_t *pstate, uint32_t phys_address, uint32_t lin_address, uint32_t flags)
{
    uint32_t dir_index = K_MEM_PDIR_INDEX(lin_address);
    uint32_t table_index = K_MEM_PTABLE_INDEX(lin_address);
    struct k_mem_pentry_t *page_entry = NULL;

    if(dir_index == K_MEM_PSTATE_DIR_INDEX)
    {
        return K_MEM_PAGING_STATUS_NOT_ALLOWED;
    }

    if(pstate == K_MEM_ACTIVE_PSTATE_ADDRESS)
    {
        page_entry = K_MEM_ACTIVE_PSTATE_DIR_ADDRESS + dir_index;
    }
    else
    {
        pstate = (struct k_mem_pstate_t *)k_mem_map_temp((uint32_t)pstate);
        page_entry = ((struct k_mem_pentry_t *)k_mem_map_temp((uint32_t)pstate->page_dir)) + dir_index;
    }

    /* TODO: handle entries marked as not present */

    if(page_entry->entry & K_MEM_PENTRY_FLAG_USED)
    {
        if(page_entry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE)
        {
            /* this entry already maps an address to a big page */
            return K_MEM_PAGING_STATUS_ALREADY_USED;
        }

        /* we temporarily map the physical page that contains this page table, so we can modify it */
        page_entry = ((struct k_mem_pentry_t *)k_mem_map_temp(page_entry->entry)) + table_index;

        if(page_entry->entry & K_MEM_PENTRY_FLAG_USED) 
        {
            /* this entry already maps an address to a normal page */
            return K_MEM_PAGING_STATUS_ALREADY_USED;
        }

        /* if we got here, it means this the page directory entry references a page 
        table and now we have a pointer to it */
    }
    else
    {
        /* if we got here the page directory entry is not being used, and we need to check what size of
        page we'll be mapping. If it's a big page, all we have to do is point the directory entry to the
        physical page and set some flags. Otherwise, we'll be mapping a normal page, and we'll need to
        first allocate a page to hold the directory's page table, map this page, then set up the physical
        page address and flags we want to actually map. */

        if(!(flags & K_MEM_PENTRY_FLAG_BIG_PAGE) && (page_entry->entry & K_MEM_PENTRY_ADDR_MASK) == 0)
        {
            /* if we got here we'll be mapping a normal page and the page directory doesn't have a page 
            table, so we allocate and map a physical page that will hold the page table we need. */

            uint32_t page_table = k_mem_alloc_page();

            if(page_table == K_MEM_INVALID_PAGE)
            {
                return K_MEM_PAGING_STATUS_NO_PTABLE;
            }

            page_entry->entry = page_table | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_READ_WRITE;
            /* now we temporarily map the physical page that contains the new page table, so we can modify it */
            page_entry = ((struct k_mem_pentry_t *)k_mem_map_temp(page_entry->entry)) + table_index;
        }
    }
    
    page_entry->entry = phys_address | flags | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED;

    return K_MEM_PAGING_STATUS_OK;
}

uint32_t k_mem_address_mapped(struct k_mem_pstate_t *pstate, uint32_t address)
{
    uint32_t dir_index = K_MEM_PDIR_INDEX(address);
    struct k_mem_pentry_t *page_entry = NULL;

    if(pstate == K_MEM_ACTIVE_PSTATE_ADDRESS)
    {
        page_entry = K_MEM_ACTIVE_PSTATE_DIR_ADDRESS + dir_index;
    }
    else
    {
        pstate = (struct k_mem_pstate_t *)k_mem_map_temp((uint32_t)pstate);
        page_entry = ((struct k_mem_pentry_t *)k_mem_map_temp((uint32_t)pstate->page_dir)) + dir_index;
    }

    if(page_entry->entry & K_MEM_PENTRY_FLAG_USED)
    {
        if(page_entry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE)
        {
            /* this entry is used and is mapping a big page */
            return 1;
        }

        /* this entry points to a page table, so we'll check that now */
        uint32_t table_index = K_MEM_PTABLE_INDEX(address);
        page_entry = ((struct k_mem_pentry_t *)k_mem_map_temp(page_entry->entry)) + table_index;

        if(page_entry->entry & K_MEM_PENTRY_FLAG_USED)
        {
            return 1;
        }
    }

    /* if the page directory entry is not used, it's guaranteed that the address is not 
    mapped in any way */
    return 0;
}

uint32_t k_mem_unmap_address(struct k_mem_pstate_t *pstate, uint32_t lin_address)
{    
    uint32_t dir_index = K_MEM_PDIR_INDEX(lin_address);
    uint32_t table_index = K_MEM_PTABLE_INDEX(lin_address);
    struct k_mem_pentry_t *page_entry = NULL;

    if(dir_index == K_MEM_PSTATE_DIR_INDEX)
    {
        return K_MEM_PAGING_STATUS_NOT_ALLOWED;
    }

    if(pstate == K_MEM_ACTIVE_PSTATE_ADDRESS)
    {
        page_entry = K_MEM_ACTIVE_PSTATE_DIR_ADDRESS + dir_index;
    }
    else
    {
        pstate = (struct k_mem_pstate_t *)k_mem_map_temp((uint32_t)pstate);
        page_entry = ((struct k_mem_pentry_t *)k_mem_map_temp((uint32_t)pstate->page_dir)) + dir_index;
    }

    if(!(page_entry->entry & K_MEM_PENTRY_FLAG_USED))
    {
        /* not being used for a big page/doesn't reference a page table */
        return K_MEM_PAGING_STATUS_NOT_PAGED;
    }

    if(!(page_entry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE))
    {
        /* if we get here, we know for sure the directory isn't mapping a big page,
        and that it has a page table allocated to it. */

        page_entry = ((struct k_mem_pentry_t *)k_mem_map_temp(page_entry->entry)) + table_index;

        if(!(page_entry->entry & K_MEM_PENTRY_FLAG_USED))
        {
            return K_MEM_PAGING_STATUS_NOT_PAGED;
        }
    }

    page_entry->entry = 0;
    k_mem_invalidate_tlb(lin_address);

    return K_MEM_PAGING_STATUS_OK;
}

struct k_mem_heap_t *k_mem_create_heap(struct k_mem_pstate_t *pstate, uint32_t start_address, uint32_t size)
{
    struct k_mem_heap_t *heap = NULL;    
    struct k_mem_block_t *header = NULL;
    uint32_t header_address = start_address + sizeof(struct k_mem_heap_t);
    uint32_t align = header_address % sizeof(struct k_mem_block_t);

    if(align)
    {
        align = sizeof(struct k_mem_block_t) - align;
        header_address += align;
    }

    uint32_t block_address = header_address + sizeof(struct k_mem_block_t);
    uint32_t used_size = block_address - start_address;

    if(used_size < size)
    {
        size -= used_size;

        if(size >= K_MEM_MIN_ALLOC_SIZE)
        {
            heap = (struct k_mem_heap_t *)start_address;
            header = (struct k_mem_block_t *)header_address;
            uint32_t next_block_page = (header_address & K_MEM_PENTRY_ADDR_MASK) + 0x1000;

            if(!k_mem_address_mapped(pstate, start_address))
            {
                /* the page that contains the heap struct is not mapped, so we need to allocate
                and map it here */
                uint32_t start_page = k_mem_alloc_page();
                k_mem_map_address(pstate, start_page, start_address, K_MEM_PENTRY_FLAG_READ_WRITE);
            }

            if(header_address >= next_block_page || next_block_page - header_address < sizeof(struct k_mem_heap_t))
            {
                /* the heap manager straddles two pages, so we need to check if the next page
                after start_address is also mapped */
                if(!k_mem_address_mapped(pstate, next_block_page))
                {
                    /* next page is not paged, so we allocate a new page and map it */
                    uint32_t next_page = k_mem_alloc_page();
                    k_mem_map_address(pstate, next_page, next_block_page, K_MEM_PENTRY_FLAG_READ_WRITE);
                }
            }

            heap->block_count = 1;
            heap->blocks = header;
            heap->last_block = header;
            heap->pstate = pstate;

            header->size = size;
            header->next = NULL;
            header->prev = NULL;
        }
    }

    return heap;
}

void *k_mem_alloc(struct k_mem_heap_t *heap, uint32_t size, uint32_t align)
{

}

uint32_t k_mem_reserve(struct k_mem_heap_t *heap, uint32_t size, uint32_t align)
{

}

void k_mem_free(struct k_mem_heap_t *heap, void *memory) 
{

}

// {
//     heap->block_count = 1;
//     heap->page_state = page_state;

//     uint32_t block_page = k_mem_alloc_page();
//     // k_mem_map_address()
// }

// void k_mem_destroy_heap(struct k_mem_heap_t *heap)
// {

// }