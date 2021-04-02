#include "k_mem.h"
#include "k_term.h"

void k_init()
{
    k_term_init();
    k_mem_init();
    k_printf("don't worry, everything is fine...\n");
}