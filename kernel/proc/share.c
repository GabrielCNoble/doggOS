#include "thread.h"


void k_proc_StartKernelThread(k_proc_thread_func_t entry_point, void *user_data)
{
    uintptr_t return_value = entry_point(user_data);
    k_proc_TerminateThread(return_value);
}

void k_proc_StartProces(k_proc_thread_func_t entry_point, void *user_data)
{
    
}
