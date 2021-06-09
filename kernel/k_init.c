#include "k_mem.h"
#include "k_term.h"
#include "k_int.h"
#include "k_dev.h"
#include <stddef.h>

extern struct k_mem_pstate_t k_mem_pstate;
void k_init()
{
    k_int_init();
    k_mem_init();
    k_dev_init();
    k_term_init();

    uint32_t test_page = k_mem_alloc_page();
    uint32_t *ptrs[] = { (uint32_t *)0xf80000, (uint32_t *)0xf81000, (uint32_t *)0xf82000, (uint32_t *)0xf83000 };

    for(uint32_t i = 0; i < 3000002; i++)
    {
        if(!(i % 2))
        {
            for(uint32_t j = 0; j < sizeof(ptrs) / sizeof(ptrs[0]); j++)
            {
                k_mem_map_address(K_MEM_ACTIVE_PSTATE, test_page, (uint32_t)ptrs[j], K_MEM_PENTRY_FLAG_READ_WRITE);        
            }
        }
        else
        {
            for(uint32_t j = 0; j < sizeof(ptrs) / sizeof(ptrs[0]); j++)
            {
                k_mem_unmap_address(K_MEM_ACTIVE_PSTATE, (uint32_t)ptrs[j]);        
            }       
        }
    }

    *ptrs[0] = 5;
    k_printf("%d\n", *ptrs[1]);

    // k_mem_map_address(K_MEM_ACTIVE_PSTATE, test_page, 0xf80000, K_MEM_PENTRY_FLAG_READ_WRITE);
    // k_mem_map_address(K_MEM_ACTIVE_PSTATE, test_page, 0xf81000, K_MEM_PENTRY_FLAG_READ_WRITE);
    // k_mem_map_address(K_MEM_ACTIVE_PSTATE, test_page, 0xf82000, K_MEM_PENTRY_FLAG_READ_WRITE);
    // k_mem_map_address(K_MEM_ACTIVE_PSTATE, test_page, 0xf83000, K_MEM_PENTRY_FLAG_READ_WRITE);

    // uint32_t *a = (uint32_t *)0xf80000;
    // uint32_t *b = (uint32_t *)0xf81000;
    // uint32_t *c = (uint32_t *)0xf82000;
    // uint32_t *d = (uint32_t *)0xf83000;

    // for(uint32_t index = 0; index < 5; index++)
    // {
    //     *a = index;
    //     k_printf("%d %d %d %d\n", *a, *b, *c, *d); 
    // }

    // k_mem_unmap_address(K_MEM_ACTIVE_PSTATE, 0xf80000);
    // k_mem_unmap_address(K_MEM_ACTIVE_PSTATE, 0xf81000);
    // k_mem_unmap_address(K_MEM_ACTIVE_PSTATE, 0xf82000);
    // k_mem_unmap_address(K_MEM_ACTIVE_PSTATE, 0xf83000);

    k_mem_free_pages(test_page);

}