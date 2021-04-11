#include "k_mem.h"
#include "k_term.h"
#include "k_int.h"
#include <stddef.h>

void k_init()
{
    k_int_init();
    k_mem_init();
    k_term_init();
    k_printf("everything is fine...\n");
    // k_mem_map_address(K_MEM_ACTIVE_PSTATE_ADDRESS, 0xb8000, 0xa00000, K_MEM_PENTRY_FLAG_READ_WRITE);

    // uint16_t *a = (uint16_t *)0x800000;
    // uint16_t *b = (uint16_t *)0xa00000;

    // *b = 5;
    // k_printf("%d\n", (uint32_t)*a);
    
    // k_mem_unmap_address(K_MEM_ACTIVE_PSTATE_ADDRESS, 0xa00000);
    // *b = 6;
    // k_printf("%d\n", (uint32_t)*b);
}