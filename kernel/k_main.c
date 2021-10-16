#include "k_main.h"
#include "../libdg/atomic/dg_atomic.h"
#include "../libdg/malloc/dg_malloc.h"
#include "k_apic.h"
#include "proc/k_proc.h"
#include "k_int.h"

extern void *k_int8_a;

void k_main()
{
    k_term_clear();

    k_proc_CreateThread(func1, 3);
    k_proc_CreateThread(func2, 0);
    k_proc_CreateThread(func3, 3);
    k_proc_CreateThread(func4, 0);
    // k_proc_CreateThread(func5);
    // k_proc_CreateThread(func6);
    k_proc_RunScheduler();
    return;
}