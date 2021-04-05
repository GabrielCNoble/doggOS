#include "k_mem.h"
#include "k_term.h"
#include "k_int.h"
#include <stddef.h>

void k_init()
{
    k_int_init();
    k_mem_init();
    k_term_init();

    uint32_t pages = k_mem_alloc_pages(3);
    k_printf("page %x allocated\n", pages);
    
    
    k_mem_free_page(pages);
    k_mem_free_page(pages + 4096);
    k_mem_free_page(pages + 8192);
    // #define COUNT 8
    // uint32_t pages[COUNT];

    // for(uint32_t page_index = 0; page_index < COUNT; page_index++)
    // {
    //     pages[page_index] = k_mem_alloc_page();
    //     k_printf("%x ", pages[page_index]);
    // }
    // k_printf("\n");

    // for(uint32_t page_index = 0; page_index < COUNT; page_index++)
    // {
    //     k_mem_free_page(pages[page_index]);
    // }

    // for(uint32_t page_index = 0; page_index < COUNT; page_index++)
    // {
    //     pages[page_index] = k_mem_alloc_page();
    //     k_printf("%x ", pages[page_index]);
    // }
}