#include "k_mem.h"
#include "k_term.h"
#include <stddef.h>


extern struct k_mem_range_t k_mem_low_range;
extern uint32_t k_mem_range_count;
uint32_t k_mem_total = 0;
extern struct k_mem_range_t k_mem_ranges[];

extern uint32_t k_mem_gdt_desc_count;
extern struct k_mem_seg_desc_t k_mem_gdt[]; 

extern struct k_mem_pentry_t k_mem_pdirs[];
struct k_mem_pentry_t *k_mem_ptables;

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
    uint32_t covered_bytes_per_bit_page = 4096 * (bits_per_page / K_MEM_USED_PAGE_BITS);
    /* to simplify mapping page addresses to its proper byte/bit, we'll allocate
    enough bytes to cover the whole 4GB address space */
    uint32_t used_page_bits = 0xffffffff / covered_bytes_per_bit_page;
    uint32_t used_page_bytes = used_page_bits / 8;
    

    /* The page directory has 1024 entries in 32 bit paging mode, and each 
    page table has 1024 page entries. */

    uint32_t total_size = free_page_header_bytes;


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
    // k_mem_pdirs = 
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

    k_mem_page_mem(0x00000000, 0x00000000, 1);
    k_mem_enable_paging();
}

uint32_t k_mem_alloc_page()
{
    uint32_t page = 0;

    if(k_mem_free_pages_cursor)
    {
        struct k_mem_pentry_t *entry = k_mem_free_pages + k_mem_free_pages_cursor;
        k_mem_free_pages_cursor--;
        uint32_t byte_index = (entry->entry >> K_MEM_SMALL_PAGE_USED_BYTE_SHIFT) & K_MEM_SMALL_PAGE_USED_BYTE_MASK;
        uint32_t bit_index = (entry->entry >> K_MEM_SMALL_PAGE_USED_BIT_SHIFT) & K_MEM_SMALL_PAGE_USED_BIT_MASK;
        k_mem_used_pages[byte_index] |= 1 << bit_index;
        page = (entry->entry & K_MEM_SMALL_PAGE_ADDR_MASK);
    }

    return page;
}

void k_mem_free_page(uint32_t page)
{
    if(page)
    {
        uint32_t byte_index = (page >> K_MEM_SMALL_PAGE_USED_BYTE_SHIFT) & K_MEM_SMALL_PAGE_USED_BYTE_MASK;
        uint32_t bit_index = (page >> K_MEM_SMALL_PAGE_USED_BIT_SHIFT) & K_MEM_SMALL_PAGE_USED_BIT_MASK;

        if(!(k_mem_used_pages[byte_index] & (1 << bit_index)))
        {
            k_mem_free_pages_cursor++;
            struct k_mem_pentry_t *entry = k_mem_free_pages + k_mem_free_pages_cursor;
            entry->entry = page;
        }
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
        page_entry->entry = physical_address & K_MEM_BIG_PAGE_ADDR_MASK;
        page_entry->entry |= K_MEM_PENTRY_FLAG_BIG_PAGE | K_MEM_PENTRY_FLAG_PRESENT;
        page_entry->entry |= K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE;

        return 1;
    }

    return 0;
}