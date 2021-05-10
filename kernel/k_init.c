#include "k_mem.h"
#include "k_term.h"
#include "k_int.h"
#include <stddef.h>

void k_init()
{
    k_int_init();
    k_mem_init();
    k_term_init();
    k_printf("boot ok!\n");
}