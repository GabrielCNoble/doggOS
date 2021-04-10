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


    uint16_t *a = (uint16_t *)0x800000;
    uint16_t *b = (uint16_t *)0xa00000;

    *a = 5;
    k_printf("%d\n", (uint32_t)*b);
}