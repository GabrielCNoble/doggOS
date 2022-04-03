#include "thread.h"
#include "../sys/syscall.h"

void k_proc_StartUserThread(k_proc_thread_func_t entry_point, void *user_data)
{
    uintptr_t return_value = entry_point(user_data);
    k_sys_SysCall(K_SYS_SYSCALL_TERMINATE_THREAD, return_value, 0, 0, 0);
}

void k_proc_StartKernelThread(k_proc_thread_func_t entry_point, void *user_data)
{
    uintptr_t return_value = entry_point(user_data);
    k_proc_TerminateThread(return_value);
}
