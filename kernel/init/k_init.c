#include "k_init.h"
#include "../k_main.h"
#include <stddef.h>

void stdcall k_Init(struct k_init_data_t *init_data)
{
    k_int_Init();
    k_mem_Init(init_data->ranges, init_data->range_count);
    k_gfx_Init();
    k_term_init();
    k_proc_Init();
    k_rng_Seed(3);
    k_apic_Init();
    k_dev_Init();
    k_dsk_Init(init_data->boot_drive);
    k_cpu_EnableInterrupts();
    k_main();
}