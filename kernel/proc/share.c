#include "thread.h"


void k_proc_StartKernelThread(k_proc_thread_func_t entry_point, void *user_data)
{
    // asm volatile ("cli\n hlt\n");
    // asm volatile ("nop\n nop\n");
    uintptr_t return_value = entry_point(user_data);
    k_proc_TerminateThread(return_value);
}
