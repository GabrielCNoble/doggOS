#include "k_main.h"
#include "../libdg/atomic/dg_atomic.h"
#include "k_apic.h"
#include "proc/k_proc.h"
#include "k_int.h"

extern void *k_int8_a;

void k_main()
{
    // k_term_clear();
    // uint32_t *ivt = (uint32_t *)0x00000000;
    // k_printf("%x\n", ivt[19]);

    // uint8_t *code = (uint8_t *)0xfe3fe;
    // for(uint32_t index = 0; index < 0x3fff; index++)
    // {
    //     k_printf("%x ", (uint32_t)code[index]);
    //     if(code[index] == 0xcf)
    //     {
    //         k_printf("found iret!\n");
    //         break;
    //     }
    // }

    // for(uint32_t index = 0; index < 256; index += 4)
    // {
    //     k_printf("%x  %x  %x  %x\n", ivt[index], ivt[index + 1], ivt[index + 2], ivt[index + 3]);
    // }

    // k_cpu_Halt();

    k_proc_CreateThread(func1, 3);
    k_proc_CreateThread(func2, 0);
    k_proc_CreateThread(func3, 3);
    k_proc_CreateThread(func4, 0);
    // k_proc_CreateThread(func5);
    // k_proc_CreateThread(func6);
    k_proc_RunScheduler();
    return;
}