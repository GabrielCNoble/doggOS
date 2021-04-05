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

extern uint32_t k_kernel_start;
extern uint32_t k_kernel_end;

uint8_t *k_mem_used_pages;
struct k_mem_pentry_t *k_mem_free_pages;
uint32_t k_mem_free_pages_cursor;
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

    uint32_t headers_per_page = 4096 / sizeof(struct k_mem_pentry_t);
    uint32_t covered_bytes_per_header_page = 4096 * headers_per_page + 4096;
    /* each block of covered_bytes_per_header_page bytes 'wastes' a page for headers,
    so that's why the size of a page gets added here. This essentially tells how many
    of those blocks + header page fits in the total memory we have */
    uint32_t free_page_headers = k_mem_total / covered_bytes_per_header_page;

    if(k_mem_total % covered_bytes_per_header_page)
    {
        free_page_headers++;
    }

    uint32_t free_page_header_bytes = free_page_headers * 4096;

    uint32_t bits_per_page = 4096 * 8;
    uint32_t covered_bytes_per_bit_page = 4096 * (bits_per_page / K_MEM_USED_PAGE_BITS) + 4096;
    /* to simplify mapping page addresses to its proper byte/bit, we'll allocate
    enough bytes to cover the whole 4GB address space */
    uint32_t used_page_bits = 0xffffffff / covered_bytes_per_bit_page;
    uint32_t used_page_bytes = used_page_bits / 8;
    
    /* The page directory has 1024 entries in 32 bit paging mode, and each 
    page table has 1024 page entries. */
    uint32_t total_size = free_page_header_bytes + used_page_bits;
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


    k_mem_free_pages = (struct k_mem_pentry_t *)((uint32_t)lowest_range->base);
    lowest_range->base += free_page_header_bytes;
    k_mem_used_pages = (uint8_t *)((uint32_t)lowest_range->base);
    lowest_range->base += used_page_bytes;
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

    k_mem_free_pages_cursor = 0xffffffff;
    
    for(uint32_t range_index = 0; range_index < k_mem_range_count; range_index++)
    {
        struct k_mem_range_t *range = k_mem_ranges + range_index;
        uint32_t page_address = (uint32_t)range->base;
        uint32_t page_count = (uint32_t)range->size / 4096;

        for(uint32_t page_index = 0; page_index < page_count; page_index++)
        {
            k_mem_free_pages_cursor++;
            struct k_mem_pentry_t *entry = k_mem_free_pages + k_mem_free_pages_cursor;
            entry->entry = page_address;
            page_address += 4096;
        }
    }

    uint32_t address = 0;

    while(address < k_mem_total)
    {
        k_mem_map_address(k_mem_pdirs, address, address, K_MEM_PENTRY_FLAG_BIG_PAGE);    
        address += 0x00400000;
    }

    k_mem_enable_paging();
}

uint32_t k_mem_alloc_page()
{
    uint32_t page_address = K_MEM_INVALID_PAGE;

    if(k_mem_free_pages_cursor != 0xffffffff)
    {
        struct k_mem_pentry_t *entry = k_mem_free_pages + k_mem_free_pages_cursor;
        k_mem_free_pages_cursor--;
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
    if(k_mem_free_pages_cursor != 0xffffffff && k_mem_free_pages_cursor >= count)
    {
        _try_again:

        uint32_t entry_count = k_mem_free_pages_cursor - count;
        for(uint32_t entry_index = 0; entry_index <= entry_count; entry_index++)
        {
            struct k_mem_pentry_t *head_entry = k_mem_free_pages + entry_index;
            struct k_mem_pentry_t *first_entry = head_entry;

            for(uint32_t next_entry_index = 1; next_entry_index < count; next_entry_index++)
            {
                struct k_mem_pentry_t *second_entry = head_entry + next_entry_index;
                uint32_t second_entry_page = second_entry->entry & K_MEM_SMALL_PAGE_ADDR_MASK;
                uint32_t first_entry_page = first_entry->entry & K_MEM_SMALL_PAGE_ADDR_MASK;
                /* likely not the ideal thing to do here. Could be more efficient to try
                both combinations, and swap the entries if those point to contiguous pages */
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
            k_mem_sort_entries(0, k_mem_free_pages_cursor + 1, k_mem_free_pages);
            entries_sorted = 1;
            goto _try_again;
        }
    }

    return page_address;
}

void k_mem_free_page(uint32_t page_address)
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
                k_mem_free_pages_cursor++;
                struct k_mem_pentry_t *entry = k_mem_free_pages + k_mem_free_pages_cursor;
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

uint32_t k_mem_map_address(struct k_mem_pentry_t *page_dir, uint32_t page, uint32_t address, uint32_t flags)
{
    struct k_mem_pentry_t *page_entry = page_dir + K_MEM_PENTRY0_INDEX(address);

    if(page_entry->entry & K_MEM_PENTRY_FLAG_USED)
    {
        if(page_entry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE)
        {
            /* this entry already maps an address to a big page */
            return K_MEM_PAGING_STATUS_PAGED;
        }

        page_entry = K_MEM_PENTRY1_ADDRESS(page_entry->entry) + K_MEM_PENTRY1_INDEX(address);

        if(page_entry->entry & K_MEM_PENTRY_FLAG_USED) 
        {
            /* this entry already maps an address to a normal page */
            return K_MEM_PAGING_STATUS_PAGED;
        }
    }

    if(!(flags & K_MEM_PENTRY_FLAG_BIG_PAGE) && (page_entry->entry & K_MEM_PENTRY_ADDR_MASK) == 0)
    {
        // uint32_t page_table = k_mem_alloc_page();

        // if(page_table == K_MEM_INVALID_PAGE)
        // {
        //     /* this will trigger swapping */
        //     return K_MEM_PAGING_STATUS_NO_PTABLE;
        // }

        // page_entry->entry = page_table | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_READ_WRITE;
        // page_entry = K_MEM_PENTRY1_ADDRESS(page_entry->entry) + K_MEM_PENTRY1_INDEX(address);
    }
    
    page_entry->entry = page | flags | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED;

    return K_MEM_PAGING_STATUS_OK;
}

uint32_t k_mem_unmap_address(struct k_mem_pentry_t *page_dir, uint32_t address)
{    
    struct k_mem_pentry_t *page_entry = page_dir + K_MEM_PENTRY0_INDEX(address);

    if(page_entry->entry & K_MEM_PENTRY_FLAG_USED)
    {
        if(!(page_entry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE))
        {
            page_entry = (struct k_mem_pentry_t *)(page_entry->entry & K_MEM_PENTRY_ADDR_MASK);
            page_entry += K_MEM_PENTRY1_INDEX(address);
        }

        if(page_entry->entry & K_MEM_PENTRY_FLAG_USED)
        {
            page_entry->entry &= ~(K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_PRESENT);
            return K_MEM_PAGING_STATUS_OK;
        }
    }

    return K_MEM_PAGING_STATUS_PAGED;
}