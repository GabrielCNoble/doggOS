#include "k_mem.h"
#include "k_term.h"
#include "k_int.h"
#include "k_dev.h"
#include <stddef.h>

void k_init()
{
    k_int_init();
    k_mem_Init();
    k_term_init();
    k_dev_init();
    k_printf("ok!\n");
}