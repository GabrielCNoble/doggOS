#include "k_main.h"
#include "../libdg/atomic/dg_atomic.h"
#include "../libdg/malloc/dg_malloc.h"
#include "../libdg/string/dg_string.h"
#include "timer/k_apic.h"
#include "proc/k_thread.h"
#include "proc/k_proc.h"
#include "k_int.h"

extern void *k_int8_a;

void k_main()
{
    _Static_assert(sizeof(uintptr_t) == sizeof(uint32_t), "Size of uintptr_t doesn't match size of uint32_t");

    // k_term_clear();

    // uint8_t src[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    // uint8_t dst[sizeof(src) / sizeof(src[0])];

    // asm volatile
    // (
    //     "nop\n"
    //     "nop\n"
    //     "nop\n"
    //     "nop\n"
    //     "nop\n"
    //     "nop\n"
    // );
    // ds_Memcpy(src + 5, src, 3);
    
    // for(uint32_t index = 0; index < sizeof(src) / sizeof(src[0]); index++)
    // {
    //     k_printf("%d\n", (uint32_t)src[index]);
    // }

    // k_cpu_Halt();

    k_proc_CreateThread(func1, NULL);
    // k_proc_CreateThread(func2, 0);
    // k_proc_CreateThread(func3, 3);
    k_proc_CreateThread(func4, NULL);
    k_proc_RunScheduler();
    return;
}