#include "k_init.h"
#include "k_main.h"
#include <stddef.h>

void stdcall k_Init(struct k_init_data_t *init_data)
{
    k_int_Init();
    k_mem_Init(init_data->ranges, init_data->range_count);
    k_gfx_Init();
    k_term_init();
    // k_dev_init();
    k_rng_Seed(3);
    k_apic_Init();
    k_main();
    k_cpu_DisableInterrupts();
    k_cpu_Halt();
}