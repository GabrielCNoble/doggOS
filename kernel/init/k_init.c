#include "k_init.h"
#include "../k_main.h"
#include <stddef.h>
#include "../../version/version.h"

extern struct k_mem_vrlist_t k_mem_virtual_ranges;

void stdcall k_Init(struct k_init_data_t *init_data)
{
    k_int_Init();
    k_mem_Init(init_data->ranges, init_data->range_count);
    k_gfx_Init();
    k_sys_Init();

    // k_term_init();
    k_timer_Init();
    k_proc_Init();
    k_rng_Seed(3);
    k_dev_Init();
    k_kb_Init();
    k_cpu_EnableInterrupts();
    k_main();
}