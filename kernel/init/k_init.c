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
    k_term_init();
    
    // for(uint32_t range_index = 0; range_index < k_mem_virtual_ranges.free_count; range_index++)
    // {
    //     struct k_mem_vrange_t *range = k_mem_virtual_ranges.ranges + range_index;
    //     k_printf("range: %x - %x\n", range->start, range->start + range->size);
    // }


    // void *mem = k_mem_AllocVirtualRange(0x1000);
    // k_mem_FreeVirtualRange(mem);
    // mem = k_mem_AllocVirtualRange(0x1000);
    // k_mem_FreeVirtualRange(mem);

    // void *mem = k_mem_AllocVirtualRange(0x1000);
    // void *mem2 = k_mem_AllocVirtualRange(0x1000);
    // k_mem_FreeVirtualRange(mem);
    // k_mem_FreeVirtualRange(mem2);


    // for(uint32_t range_index = 0; range_index < k_mem_virtual_ranges.free_count; range_index++)
    // {
    //     struct k_mem_vrange_t *range = k_mem_virtual_ranges.ranges + range_index;
    //     k_printf("range: %x - %x\n", range->start, range->start + range->size);
    // }

    // // k_printf("cock\n");
    // mem = k_mem_AllocVirtualRange(0x1000);
    // // // k_printf("%x\n", (uint32_t)mem);
    // mem2 = k_mem_AllocVirtualRange(0x1000);
    // // // k_printf("%x\n", (uint32_t)mem2);
    // k_mem_FreeVirtualRange(mem);
    // k_mem_FreeVirtualRange(mem2);

    // for(uint32_t range_index = 0; range_index < k_mem_virtual_ranges.free_count; range_index++)
    // {
    //     struct k_mem_vrange_t *range = k_mem_virtual_ranges.ranges + range_index;
    //     k_printf("range: %x - %x\n", range->start, range->start + range->size);
    // }

    // mem = k_mem_AllocVirtualRange(0x1000);
    // mem2 = k_mem_AllocVirtualRange(0x1000);
    // k_mem_FreeVirtualRange(mem);
    // k_mem_FreeVirtualRange(mem2);

    // uint32_t *a = (uint32_t *)k_mem_AllocVirtualRange(0x1000);
    // k_printf("memory at: %x\n", (uint32_t)a);
    // uint32_t *b = 0x0a000000;
    // uintptr_t page = k_mem_AllocPhysicalPage(0);
    // k_mem_MapLinearAddress((uintptr_t)a, page, K_MEM_PENTRY_FLAG_READ_WRITE);
    // k_mem_MapLinearAddress((uintptr_t)b, page, K_MEM_PENTRY_FLAG_READ_WRITE);
    // *a = 15;
    // k_printf("read a through b: %d\n", *b);

    // k_mem_UnmapLinearAddress((uintptr_t)a);
    // k_mem_FreeVirtualRange((void*)a);
    // a = (uint32_t *)k_mem_AllocVirtualRange(0x1000);
    // k_printf("memory at: %x\n", (uint32_t)a);
    // k_mem_MapLinearAddress((uintptr_t)a, page, K_MEM_PENTRY_FLAG_READ_WRITE);
    // *a = 25;
    // k_printf("read a through b: %d\n", *b);

    // k_mem_UnmapLinearAddress((uintptr_t)a);
    // k_mem_FreeVirtualRange((void*)a);
    // a = (uint32_t *)k_mem_AllocVirtualRange(0x1000);
    // uint32_t *c = (uint32_t *)k_mem_AllocVirtualRange(0x1000);
    // k_mem_MapLinearAddress((uintptr_t)a, page, K_MEM_PENTRY_FLAG_READ_WRITE);
    // k_mem_MapLinearAddress((uintptr_t)c, page, K_MEM_PENTRY_FLAG_READ_WRITE);
    // *c = 45;
    // k_printf("read c through a: %d\n", *a);


    // k_mem_UnmapLinearAddress((uintptr_t)a);
    // k_mem_UnmapLinearAddress((uintptr_t)c);
    // k_mem_FreeVirtualRange((void*)a);
    // k_mem_FreeVirtualRange((void*)c);
    // a = (uint32_t *)k_mem_AllocVirtualRange(0x1000);
    // uint32_t *c = (uint32_t *)k_mem_AllocVirtualRange(0x1000);
    // k_mem_MapLinearAddress((uintptr_t)a, page, K_MEM_PENTRY_FLAG_READ_WRITE);
    // k_mem_MapLinearAddress((uintptr_t)c, page, K_MEM_PENTRY_FLAG_READ_WRITE);
    // *c = 45;
    // k_printf("read c through a: %d\n", *a);



    k_cpu_DisableInterrupts();
    k_cpu_Halt();

    k_timer_Init();
    k_proc_Init();
    k_rng_Seed(3);
    k_dev_Init();
    k_term_clear();
    // k_dsk_Init(init_data->boot_drive);
    k_cpu_EnableInterrupts();
    k_main();
}