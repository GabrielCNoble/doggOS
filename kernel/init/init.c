#include "init.h"
#include "../k_main.h"
#include "../../version/version.h"
#include "../dev/dsk.h"
#include "../dev/boot/boot.h"
#include "../boot/k_bootinfo.h"

extern struct k_mem_vrlist_t k_mem_virtual_ranges;

void stdcall k_Init(struct k_init_data_t *init_data)
{
    union k_boot_info_t boot_info;
    k_int_Init();
    k_mem_Init(init_data->ranges, init_data->range_count);
    k_gfx_Init();
    k_sys_Init();
    k_timer_Init();
    k_proc_Init();
    k_rng_Seed(3);
    k_dev_Init();
    k_cpu_EnableInterrupts();
    k_BOOT_Read(NULL, NULL, &boot_info);
    k_sys_TerminalClear();
    k_sys_TerminalPrintf("%d\n", boot_info.kernel_start);
    k_main();
}